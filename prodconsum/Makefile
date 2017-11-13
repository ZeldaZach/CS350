all: producer consumer

producer: producer_numbers.c
	gcc producer_numbers.c -o producer_numbers

consumer: consumer_numbers.c
	gcc consumer_numbers.c -o consumer_numbers

clean:
	rm -f producer_numbers consumer_numbers
