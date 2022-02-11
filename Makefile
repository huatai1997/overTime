.PHONY:all

all:
	make -C ./src/
	cp ./src/main ./getOverTime

.PHONY:clean

clean:
	make -C ./src/ clean
	rm -f ./getOverTime
