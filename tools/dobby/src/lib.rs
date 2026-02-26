
use rustls::client::danger::{HandshakeSignatureValid, ServerCertVerified, ServerCertVerifier};
use rustls::pki_types::{CertificateDer, ServerName, UnixTime};
use rustls::{DigitallySignedStruct, Error as RustlsError, SignatureScheme};
use std::time::Duration;
use rumqttc::{Client, Connection, Incoming, QoS, Event};
use std::time::Instant;
use std::io::{self, BufRead, Write};
use std::thread;

#[derive(Debug)]
pub struct NoCertificateVerification;

impl ServerCertVerifier for NoCertificateVerification {
    fn verify_server_cert(
        &self,
        _end_entity: &CertificateDer<'_>,
        _intermediates: &[CertificateDer<'_>],
        _server_name: &ServerName<'_>,
        _ocsp_response: &[u8],
        _now: UnixTime,
    ) -> Result<ServerCertVerified, RustlsError> {
        Ok(ServerCertVerified::assertion())
    }

    fn verify_tls12_signature(
        &self,
        _message: &[u8],
        _cert: &CertificateDer<'_>,
        _dss: &DigitallySignedStruct,
    ) -> Result<HandshakeSignatureValid, RustlsError> {
        Ok(HandshakeSignatureValid::assertion())
    }

    fn verify_tls13_signature(
        &self,
        _message: &[u8],
        _cert: &CertificateDer<'_>,
        _dss: &DigitallySignedStruct,
    ) -> Result<HandshakeSignatureValid, RustlsError> {
        Ok(HandshakeSignatureValid::assertion())
    }

    fn supported_verify_schemes(&self) -> Vec<SignatureScheme> {
        vec![
            SignatureScheme::ECDSA_NISTP256_SHA256,
            SignatureScheme::ECDSA_NISTP384_SHA384,
            SignatureScheme::ECDSA_NISTP521_SHA512,
            SignatureScheme::RSA_PSS_SHA256,
            SignatureScheme::RSA_PSS_SHA384,
            SignatureScheme::RSA_PSS_SHA512,
            SignatureScheme::RSA_PKCS1_SHA256,
            SignatureScheme::RSA_PKCS1_SHA384,
            SignatureScheme::RSA_PKCS1_SHA512,
            SignatureScheme::ED25519,
            SignatureScheme::ED448,
        ]
    }
}

pub fn convert_millis_to_hms(millis: u32) -> (u64, u64, u64, u64) {
    let duration = Duration::from_millis(millis as u64);
    let total_seconds = duration.as_secs();

    let days = total_seconds / 86400; // 60 * 60 * 24
    let remaining_seconds = total_seconds % 86400;

    let hours = remaining_seconds / 3600; // 60 * 60
    let remaining_seconds = remaining_seconds % 3600;

    let minutes = remaining_seconds / 60;
    let seconds = remaining_seconds % 60;

    (days, hours, minutes, seconds)
}

pub struct Device {
    pub name: String,
    pub uptime: u32,
}

pub fn scan_for_devices_for_seconds(mqtt_client: Client, mut mqtt_connection: Connection, maintopic: String, time_to_scan_s: u64) {
     let heartbeat_topic: String = format!("{}/+/heartbeat", maintopic);

    mqtt_client.subscribe(heartbeat_topic, QoS::AtMostOnce).unwrap();
    let mut devices: Vec<Device> = Vec::new();
    let time_limit = Duration::from_secs(time_to_scan_s);
    let start_time = Instant::now();

    for (_i, notification) in mqtt_connection.iter().enumerate() {
        if start_time.elapsed() >= time_limit {
            println!("Time limit reached. Exiting.");
            break;
        }

        // if args.verbose {
        //     println!("Notification[{}] = {:?}", i, notification);
        // }

        if let Ok(Event::Incoming(Incoming::Publish(publish))) = notification {
            // println!("[{}] {}: {}", i, publish.topic, String::from_utf8_lossy(&publish.payload));
            let parts: Vec<_> = publish.topic.split("/").collect();
            let device = parts[1];
            let uptime = String::from_utf8_lossy(&publish.payload).parse::<u32>().expect("Uptime is not a number");
            
        let mut is_new_device: bool = true;    

        for d in &mut devices {
            if d.name == device.to_string() {
                is_new_device = false;
                d.uptime = uptime;
                // println!("Updated uptime for {} to {}", device, uptime);
            }
        }
        if is_new_device {
            let new_device = Device {
                name: device.to_string(),
                uptime: uptime,
            };

            devices.push(new_device);
            let (d, h, m ,s) = convert_millis_to_hms(uptime);
            println!("💡 {:16} 🥾 {}d {}h {}m {}s", device, d, h, m, s);
        }
        }
    }
}

pub fn show_log_from_device(mqtt_client: Client, mut mqtt_connection: Connection, maintopic: String, device: String) {
    let log_topic: String = format!("{}/{}/log", maintopic, device);
    mqtt_client.subscribe(log_topic, QoS::AtMostOnce).unwrap();

    for (_i, notification) in mqtt_connection.iter().enumerate() {
        if let Ok(Event::Incoming(Incoming::Publish(publish))) = notification {
            let log_message = String::from_utf8_lossy(&publish.payload);
            println!("{}", log_message);
        }
    }
}

pub fn start_interactive(mqtt_client: Client, mut mqtt_connection: Connection, maintopic: String, device: String) {
    let log_topic: String = format!("{}/{}/log", maintopic, device);
    let cmd_topic: String = format!("{}/{}/command", maintopic, device);
    mqtt_client.subscribe(log_topic, QoS::AtMostOnce).unwrap();

    let log_thread = thread::spawn(move || {
        for notification in mqtt_connection.iter() {
            if let Ok(Event::Incoming(Incoming::Publish(publish))) = notification {
                let log_message = String::from_utf8_lossy(&publish.payload);
                if log_message.contains("RESPONSE") {
                    let (_head, cmd_response) = log_message.split_once("]").expect(&format!("Wrong response format: {}", log_message));
                    println!("{}", cmd_response);
                }
            }
        }
    });

    println!("Interactive mode started. Type commands and press enter. Type 'exit' to quit.");
    let stdin = io::stdin();
    let prompt = format!("{}> ", device);
    print!("{}", prompt);
    io::stdout().flush().expect("Failed to flush stdout");
    for line in stdin.lock().lines() {
        match line {
            Ok(command) => {
                let command = command.trim();
                if command.eq_ignore_ascii_case("exit") {
                    break;
                }
                if let Err(err) = mqtt_client.publish(&cmd_topic, QoS::AtMostOnce, false, command) {
                    eprintln!("Failed to publish command: {}", err);
                }
                if command.eq_ignore_ascii_case("help") {
                    println!("Type \"1\" to get list of commands from device")
                }
                // prepare for next command
                if ! command.is_empty() {
                    thread::sleep(Duration::from_millis(500));
                }
                print!("{}", prompt);
                io::stdout().flush().expect("Failed to flush stdout");
            }
            Err(err) => {
                eprintln!("Failed to read from stdin: {}", err);
                break;
            }
        }
    }
    drop(log_thread);
}