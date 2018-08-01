from kafka import KafkaProducer 
from kafka.errors import KafkaError
 
producer = KafkaProducer(bootstrap_servers=['babel.udl.cat:6667']) 
topic = "fran"
producer.send(topic, b'test message')
producer.flush()

