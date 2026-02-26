use rumqttc::{MqttOptions, Client, Transport, TlsConfiguration};
use std::time::Duration;
use clap::Parser;
use std::fs;
use std::sync::Arc;
use rustls::ClientConfig;

// use chrono::{TimeZone, Utc, NaiveDateTime};
use dobby::{NoCertificateVerification, scan_for_devices_for_seconds, show_log_from_device};

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    /// Name of the person to greet
    #[arg(short='b', long, default_value_t = String::from("cederlov.com"), help = "Broker address")]
    broker: String,

    #[arg(short='p', long, default_value_t = 38883, help = "Broker port")]
    port: u16,

    #[arg(short='t', long, default_value_t = String::from("test"), help = "Main topic")]
    topic: String,

    #[arg(short='v', long, help = "Verbose output")]
    verbose: bool,

    #[arg(long, help = "Allow older CA-certificates to be used")]
    insecure: bool,

    #[arg(short='i', long, help = "Start interactive prompt for selected device")]
    interactive: String,

    #[arg(short='s', long, default_value_t = false, help = "Scan for devices")]
    scan: bool,

    #[arg(short='l', long, help = "Show log output for selected device")]
    log: String,
    
}

fn main() {
    let args = Args::parse();

    let mut mqtt_options = MqttOptions::new("rumqtt-sync", args.broker, args.port);
    mqtt_options.set_keep_alive(Duration::from_secs(5));

    if args.insecure {
        // eprintln!("WARNING: TLS certificate verification is disabled (--insecure)");
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

    let (mqtt_client, mqtt_connection) = Client::new(mqtt_options, 10);
        // client.publish(topic, QoS::AtLeastOnce, false, message).unwrap();

    if args.scan {
        scan_for_devices_for_seconds(mqtt_client, mqtt_connection, args.topic, 10);
    }
    else if args.interactive {
        println!("Start interactive mode for device {}", args.interactive);
    }
    else if args.log {
        println!("--- Log for device {} ---", args.log);
        show_log_from_device(mqtt_client, mqtt_connection, args.topic, args.log);
    }
    else {
        println!("Nothing to do...")
    }

   
}













