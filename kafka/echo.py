from kafka import KafkaConsumer
from kafka import KafkaProducer

topic_receive = 'test'
topic_send = 'test2'

# consumer = KafkaConsumer(topic_receive)
consumer = KafkaConsumer(topic_receive, bootstrap_servers='babel.udl.cat:6667')
producer = KafkaProducer(bootstrap_servers='babel.udl.cat:6667')

for msg in consumer:
	producer.send(topic_send, msg)
	print (msg)
	producer.flush()