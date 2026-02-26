use rumqttc::{MqttOptions, QoS, Client, Event, Incoming, Transport, TlsConfiguration};
// use tokio::{task, time};
use std::time::Duration;
use std::time::Instant;
// use std::error::Error;
use clap::Parser;
use std::fs;
use std::sync::Arc;
use rustls::client::danger::{HandshakeSignatureValid, ServerCertVerified, ServerCertVerifier};
use rustls::pki_types::{CertificateDer, ServerName, UnixTime};
use rustls::{ClientConfig, DigitallySignedStruct, Error as RustlsError, SignatureScheme};
// use chrono::{TimeZone, Utc, NaiveDateTime};

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    /// Name of the person to greet
    #[arg(short='b', long, default_value_t = String::from("cederlov.com"))]
    broker: String,

    #[arg(short='p', long, default_value_t = 38883)]
    port: u16,

    #[arg(short='t', long, default_value_t = String::from("test"))]
    topic: String,

    // #[arg(short='m', long)]
    // message: String,

    #[arg(short='v', long)]
    verbose: bool,

    #[arg(long)]
    insecure: bool,
}

#[derive(Debug)]
struct NoCertificateVerification;

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

struct Device {
    name: String,
    uptime: u32,
}

// impl Device {
//     fn new(&self) {
//          self;
//     }
// }

// fn format_time(unixtime: u32) {
//     // 1. Convert u32 to NaiveDateTime (treating as seconds)
//     let ndt = NaiveDateTime::from_timestamp_opt(unixtime as i64, 0).unwrap();
    
//     // 2. Format using strftime
//     let formatted = ndt.format("%Y-%m-%d %H:%M:%S").to_string();
//     println!("{}", formatted); // Output: 2023-03-15 13:20:00
// }

fn convert_millis_to_hms(millis: u32) -> (u64, u64, u64, u64) {
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

fn main() {
    let args = Args::parse();

    let mut mqtt_options = MqttOptions::new("rumqtt-sync", args.broker, args.port);
    mqtt_options.set_keep_alive(Duration::from_secs(5));

    if args.insecure {
        eprintln!("WARNING: TLS certificate verification is disabled (--insecure)");
        let tls_config = ClientConfig::builder()
            .dangerous()
            .with_custom_certificate_verifier(Arc::new(NoCertificateVerification))
            .with_no_client_auth();
        mqtt_options.set_transport(Transport::Tls(TlsConfiguration::Rustls(Arc::new(tls_config))));
    } else {
        let ca = fs::read("/home/tc/cederlov_mqtt_ca.crt").expect("Failed to read CA certificate");
        let tls_config = TlsConfiguration::Simple {
            ca,
            client_auth: None,
            alpn: None,
        };
        mqtt_options.set_transport(Transport::Tls(tls_config));
    }

    let (mqtt_client, mut mqtt_connection) = Client::new(mqtt_options, 10);

    let heartbeat_topic: String = format!("{}/+/heartbeat", args.topic);


    mqtt_client.subscribe(heartbeat_topic, QoS::AtMostOnce).unwrap();

    // client.publish(topic, QoS::AtLeastOnce, false, message).unwrap();


    let mut devices: Vec<Device> = Vec::new();
    // let mut uptimes: Vec<u32> = Vec::new();

    let time_limit = Duration::from_secs(10);
    let start_time = Instant::now();

    for (i, notification) in mqtt_connection.iter().enumerate() {
        if start_time.elapsed() >= time_limit {
            println!("Time limit reached. Exiting.");
            break; // Exit the loop
        }



        if args.verbose {
            println!("Notification[{}] = {:?}", i, notification);
        }

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













