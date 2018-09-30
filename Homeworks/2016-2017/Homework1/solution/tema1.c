#include <stdio.h>
#define FIRST_BYTE 0xFF
#define BYTE_SIZE 8
#define MASK(n) (n == 0 ? 0 : n == 32 ? -1 : (~0) ^ ((1 << (32 - n)) - 1))

unsigned make_ip(int ip_vect[4]) {
	int ip = 0;
	int i;
	for (i = 0; i < 4; ++i) {
		ip = (ip << 8) | ip_vect[i]; 
	}
	return ip;
}

void print_ip_addr(int ip, char * format) {
	int i = 0;
	for (i = 3; i > 0; --i) {
		printf(format, (ip >> (i * BYTE_SIZE)) & FIRST_BYTE);
		printf(".");
	}
	printf(format, ip & FIRST_BYTE);
}

void print_byte(unsigned char byte) {
	int i;
	for(i = 7; i >= 0; --i) {
		printf("%d", ((1 << i) & byte) != 0);
	}
}

void print_ip_bin(int ip) {
	int i = 0;
	unsigned char byte;
	for (i = 3; i > 0; --i) {
		byte = (ip >> (i * BYTE_SIZE)) & FIRST_BYTE;
		print_byte(byte);
		printf(".");
	}
	byte = ip & FIRST_BYTE;
	print_byte(byte);
}

void print_byte_32(unsigned char byte) {
	static char * map32 = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
	int fivebit1, fivebit2;
	
	fivebit1 = (byte >> 5) & ((1<<5) - 1);
	fivebit2 = byte & ((1<<5) - 1);
	if (byte == 0) {
		printf("0");
	} else {
		if (fivebit1 != 0)
			printf("%c", map32[fivebit1]);
		printf("%c", map32[fivebit2]);	
	}
}

void print_ip_32(int ip) {
	int i = 0;
	unsigned char byte;
	for (i = 3; i > 0; --i) {
		byte = (ip >> (i * BYTE_SIZE)) & FIRST_BYTE;
		print_byte_32(byte);
		printf(".");
	}
	byte = ip & FIRST_BYTE;
	print_byte_32(byte);
}

void task0(int ip) {
	printf("-0 ");
        print_ip_addr(ip, "%d");
        printf("\n");
}

void task1(int mask){
	printf("-1 ");
	print_ip_addr(mask, "%d");
	printf("\n");
}

void task2(int ip, int mask) {
	printf("-2 ");
	print_ip_addr(ip & mask, "%d");
	printf("\n");
}

void task3(int ip, int mask) {
	printf("-3 ");
	print_ip_addr((ip | (~mask)), "%d");
	printf("\n");
}

int task4(int ip1, int mask, int ip2) {
	
	if ((ip1 & mask) == (ip2 & mask)) {
		printf("-4 da\n");
		return 0;
	} else {
		printf("-4 nu\n");
		return 1;
	}
}

void task5(int ip1, int ip2) {
	printf("-5 ");
	print_ip_addr(ip1, "%X");
	printf(" ");
	print_ip_addr(ip2, "%o");
	printf("\n");
}

void task6(int search, int ip, int n) {
	int i;
	int ip1, ip_vec[4], mask_len, mask;
	printf("-6");
	if (search) {
		for (i = 1; i <= n; ++i){
			scanf("%d.%d.%d.%d/%d",
				ip_vec, ip_vec + 1, ip_vec + 2, ip_vec + 3, &mask_len);
			ip1 = make_ip(ip_vec);
			mask = MASK(mask_len);
			if ((ip1 & mask) == (ip & mask))
				printf(" %d", i);
		}
		printf("\n");
	} else {
		for (i = 1; i <= n; ++i)
			scanf("%d.%d.%d.%d/%d",
				ip_vec, ip_vec + 1, ip_vec + 2, ip_vec + 3, &mask_len);
		printf(" 0\n");
	}
}

void task7(int ip1, int ip2) {
	printf("-7 ");
	print_ip_bin(ip1);
	printf(" ");
	print_ip_bin(ip2);
	printf("\n");
}

void task8(int ip1, int ip2) {
	printf("-8 ");
	print_ip_32(ip1);
	printf(" ");
	print_ip_32(ip2);
	printf("\n");
}

int main() {
	int n, i, ip_vec1[4], ip_vec2[4], mask_len, num_networks;
	int ip1, ip2, mask;
	int task4_return_val;
	scanf("%d", &n);
	
	for (i = 1; i <= n; ++i) {
		printf("%d\n", i);
		scanf("%d.%d.%d.%d/%d ",
			   ip_vec1, ip_vec1 + 1, ip_vec1 + 2, ip_vec1 + 3, &mask_len);
		ip1 = make_ip(ip_vec1);
		mask = MASK(mask_len);
		scanf("%d.%d.%d.%d %d",
			   ip_vec2, ip_vec2 + 1, ip_vec2 + 2, ip_vec2 + 3, &num_networks);
		ip2 = make_ip(ip_vec2);
		task0(ip1);
		task1(mask);
		task2(ip1, mask);
		task3(ip1, mask);
		task4_return_val = task4(ip1, mask, ip2);
		task5(ip1, ip2);
		task6(task4_return_val, ip2, num_networks);
		task7(ip1, ip2);
		task8(ip1, ip2);
	}

	return 0;
}
