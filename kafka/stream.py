import os
packages = "org.apache.spark:spark-sql-kafka-0-10_2.11:2.3.1"

os.environ["PYSPARK_SUBMIT_ARGS"] = (
    "--packages {0} pyspark-shell".format(packages)
)

from pyspark.sql import SparkSession
from pyspark.sql.functions import explode
from pyspark.sql.functions import split

spark = SparkSession \
    .builder \
    .appName("StructuredKafkaWordCount") \
    .getOrCreate()

df = spark \
  .readStream \
  .format("kafka") \
  .option("kafka.bootstrap.servers", "babel.udl.cat:6667") \
  .option("subscribe", "test") \
  .load() \
  .selectExpr("CAST(key AS STRING)", "CAST(value AS STRING)") \
  .writeStream \
  .format("console") \
  .trigger(continuous="1 second") \
  .start() \
  .awaitTermination()
