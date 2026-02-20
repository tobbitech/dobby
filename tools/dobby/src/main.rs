use rumqttc::{MqttOptions, AsyncClient, QoS};
use tokio::{task, time};
use std::time::Duration;
use std::error::Error;
// use std::process;

// fn sync_publish() {
//     let mut mqttoptions = MqttOptions::new("rumqtt-sync", "localhost", 1883);
//     mqttoptions.set_keep_alive(Duration::from_secs(5));

//     let (mut client, mut connection) = Client::new(mqttoptions, 10);
//     client.subscribe("hello/rumqtt", QoS::AtMostOnce).unwrap();
//     thread::spawn(move || for i in 0..10 {
//     client.publish("hello/rumqtt", QoS::AtLeastOnce, false, i.to_string() ).unwrap();
//     thread::sleep(Duration::from_millis(1000));
//     });

//     // Iterate to poll the eventloop for connection progress
//     for (i, notification) in connection.iter().enumerate() {
//         println!("Notification = {:?}", notification);
//     }
// }

async fn async_publish() {
    let mut mqttoptions = MqttOptions::new("rumqtt-async", "localhost", 1883);
    mqttoptions.set_keep_alive(Duration::from_secs(5));

    let (client, mut eventloop) = AsyncClient::new(mqttoptions, 10);
    client.subscribe("hello/rumqtt", QoS::AtMostOnce).await.unwrap();

    task::spawn(async move {
        for i in 0..10 {
            client.publish("hello/rumqtt", QoS::AtLeastOnce, false, i.to_string() ).await.unwrap();
            time::sleep(Duration::from_millis(100)).await;
        }
    });

    while let Ok(notification) = eventloop.poll().await {
        println!("Received = {:?}", notification);
    }
}

// fn main() {
//     async_publish()

// }


#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>> {
    async_publish().await;
    Ok(())
}














