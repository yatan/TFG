PPCAS run:

	./run.sh ../examples/sdr_100 10 127.0.0.1 10 sdr_100

t_coffe run:

	./run.sh -i ../examples/sdr_100 -m 127.0.0.1 -s 127.0.0.1

OUTPUT Execution:

	./t_coffee /home/frb2/BDT-Coffee/examples/sdr_100 -extend_mode very_fast_triplet -dp_mode myers_miller_pair_wise -table_name sdr_100_bdtc_1 -cassandra_seed 127.0.0.1 -chunk_size 1



Using rrm_100 created data:

	./t_coffee /home/frb2/BDT-Coffee/examples/rrm_100 -extend_mode very_fast_triplet -dp_mode myers_miller_pair_wise -table_name rrm_100 -cassandra_seed 127.0.0.1 -chunk_size 10


Kafka ( Docker )
-----------------------------------

Run docker:

	docker run -v `pwd`:/home/frb2/host -p 4040:4040 -p 8888:8888 -p 23:22 -p 9091:9091 -p 9092:9092 -p 2181:2181 -ti --privileged yannael/kafka-sparkstreaming-cassandra

Start services:

	startup-script.sh

(Optional) Run notebook:

	notebook

Rel: https://www.tutorialspoint.com/apache_kafka/apache_kafka_basic_operations.htm

Create Topic:
	
	kafka-topics.sh --create --zookeeper localhost:2181 --replication-factor 1 --partitions 1 --topic Hello-Kafka

Consumer (Receive):

	kafka-console-consumer.sh --zookeeper localhost:2181 --topic Hello-Kafka --from-beginning
