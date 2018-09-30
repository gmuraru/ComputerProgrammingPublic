CFLAGS := -Wall -Wextra -Werror -std=gnu99 -g

EXEC := paint_simulator

build: $(EXEC)

$(EXEC): $(EXEC).c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXEC)
