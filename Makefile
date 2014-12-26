
ctlrobot: ctlrobot.o
	$(CC) -o ctlrobot ctlrobot.o -lusb

clean:
	rm -f ctlrobot ctlrobot.o
