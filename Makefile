all: slam_daemon
slam_start.o:      src/slam_start.c
	gcc -fPIC -g -c src/slam_start.c
read_config.o:      src/read_config.c
	gcc -fPIC -g -c src/read_config.c
logger.o:      src/auxiliary/logger.c
	gcc -fPIC -g -c src/auxiliary/logger.c
go_daemon.o:      src/auxiliary/go_daemon.c
	gcc -fPIC -g -c src/auxiliary/go_daemon.c
main_entry.o:      src/main_entry.c
	gcc -fPIC -g -c src/main_entry.c -lpthread
socket_ops.o:      src/socket_ops.c
	gcc -fPIC -g -c src/socket_ops.c
socket_readers.o: src/socket_readers.c
	gcc -fPIC -g -c src/socket_readers.c -lpthread
socket_aggregators.o: src/socket_aggregators.c
	gcc -fPIC -g -c src/socket_aggregators.c -lpthread
syslog_parser.o: src/parsers/syslog_parser.c
	gcc -fPIC -g -c src/parsers/syslog_parser.c -lz
kafka_producer.o: src/kafka_producer.c
	gcc -fPIC -g -c src/kafka_producer.c

slam_daemon: slam_start.o read_config.o logger.o go_daemon.o socket_ops.o socket_readers.o socket_aggregators.o syslog_parser.o kafka_producer.o main_entry.o
	gcc -g -o slam_daemon *.o -lpthread -lz
	sudo mv slam_daemon /usr/bin/

clean:
	rm -f *.o;rm -f *.gch


