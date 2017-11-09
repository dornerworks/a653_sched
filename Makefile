TARGET := a653_sched

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) -o $@ $^ \
	-luuid -lxenctrl -lxenstore

clean:
	-$(RM) $(TARGET) *.o
