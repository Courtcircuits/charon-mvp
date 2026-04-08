#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/select.h>
#include <arpa/inet.h>

#define TUN_DEVICE "/dev/net/tun"
#define MTU 1380
#define BUF_SIZE 2048

// IDEA:
// 1. Run the setup script : `sudo ./setup.sh`
// 2. Compile this code : `gcc main.c -o tun_example`
// 3. Run the program : `sudo ./tun_example`
//
// SUMMARY:
// This code creates a TUN interface named "tun0" and waits for packets to arrive. When a packet is read from the TUN interface, it prints the number of bytes read. You can process the packet data in the buffer as needed.
//


// the encoding will be really easy, we will just replace each occurences of 'dog'
// with 'cat' for the sake of the exercise.
int encode_packet(const uint8_t *plaintext, int plain_len, uint8_t *enc_buf, int enc_buf_size) {
		int enc_len = 0;
		for (int i = 0; i < plain_len && enc_len < enc_buf_size; i++) {
				if (i + 2 < plain_len && plaintext[i] == 'd' && plaintext[i + 1] == 'o' && plaintext[i + 2] == 'g') {
						if (enc_len + 3 > enc_buf_size) break; // Check buffer size
						enc_buf[enc_len++] = 'c';
						enc_buf[enc_len++] = 'a';
						enc_buf[enc_len++] = 't';
						i += 2; // Skip 'dog'
				} else {
						enc_buf[enc_len++] = plaintext[i];
				}
		}
		return enc_len;
}

// The decoding will be the reverse of encoding, replacing 'cat' with 'dog'.
int decode_packet(const uint8_t *enc_buf, int enc_len, uint8_t *dec_buf, int dec_buf_size) {
		int dec_len = 0;
		for (int i = 0; i < enc_len && dec_len < dec_buf_size; i++) {
				if (i + 2 < enc_len && enc_buf[i] == 'c' && enc_buf[i + 1] == 'a' && enc_buf[i + 2] == 't') {
						if (dec_len + 3 > dec_buf_size) break; // Check buffer size
						dec_buf[dec_len++] = 'd';
						dec_buf[dec_len++] = 'o';
						dec_buf[dec_len++] = 'g';
						i += 2; // Skip 'cat'
				} else {
						dec_buf[dec_len++] = enc_buf[i];
				}
		}
		return dec_len;
}

void print_hex(const uint8_t *data, int len) {
		for (int i = 0; i < len; i++) {
				printf("%c", data[i]);
		}
		printf("\n");
}

int udp_open(uint16_t local_port) {
		int fd = socket(AF_INET, SOCK_DGRAM, 0);
		if (fd < 0) {
				perror("socket");
				return -1;
		}

		struct sockaddr_in local_addr = {0};
		local_addr.sin_family = AF_INET;
		local_addr.sin_addr.s_addr = INADDR_ANY;
		local_addr.sin_port = htons(local_port);

		if (bind(fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
				perror("bind");
				close(fd);
				return -1;
		}
		return fd;
}


int tun_open(const char *ifname) {
		struct ifreq ifr = {0};
		int fd = open(TUN_DEVICE, O_RDWR);
		if (fd < 0) {
				perror("open tun");
				return -1;
		}

		ifr.ifr_flags = IFF_TUN | IFF_NO_PI; // IFF_NO_PI: no packet information
		strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

		if (ioctl(fd, TUNSETIFF, &ifr) < 0) {
				perror("ioctl tun");
				close(fd);
				return -1;
		}
		return fd;
}

void tunnel(int tun_fd, int udp_fd, struct sockaddr_in *peer_addr) {
		uint8_t plain_buf[BUF_SIZE];
		uint8_t enc_buf[BUF_SIZE];
		fd_set read_fds;
		int max_fd = ((tun_fd > udp_fd) ? tun_fd : udp_fd) + 1; // Check which fd is larger for select -> the lowest fd is the sucessful call not currently open for the process

		while (1) {
				FD_ZERO(&read_fds);
				FD_SET(tun_fd, &read_fds); // incoming packets from OS
				FD_SET(udp_fd, &read_fds); // incoming packets from peer

				if (select(max_fd, &read_fds, NULL, NULL, NULL) < 0) { // Wait for either tun_fd or udp_fd to have data to read
						perror("select");
						break;
				}

				if (FD_ISSET(tun_fd, &read_fds)) { // SEND PATH : OS -> Encode -> peer 
						ssize_t nread = read(tun_fd, plain_buf, sizeof(plain_buf));
						if (nread < 0) {
								perror("read tun");
								break;
						}

						print_hex(plain_buf, nread); // Print the raw packet data in hex format

						int enc_len = encode_packet(plain_buf, nread, enc_buf, sizeof(enc_buf));
						if (sendto(udp_fd, enc_buf, enc_len, 0, (struct sockaddr *)peer_addr, sizeof(*peer_addr)) < 0) {
								perror("sendto peer");
								break;
						}
				}

				if (FD_ISSET(udp_fd, &read_fds)) { // RECV PATH : peer -> Decode -> OS
						socklen_t addr_len = sizeof(*peer_addr);
						ssize_t nread = recvfrom(udp_fd, enc_buf, sizeof(enc_buf), 0, (struct sockaddr *)peer_addr, &addr_len);
						if (nread < 0) {
								perror("recvfrom peer");
								break;
						}

						print_hex(enc_buf, nread); // Print the raw packet data in hex format
						int dec_len = decode_packet(enc_buf, nread, plain_buf, sizeof(plain_buf));


						if(write(tun_fd, enc_buf, nread) < 0) {
								perror("write tun");
								break;
						}
				}
		}
}


struct config {
		char *peer_ip;
		int peer_port;
		int local_port;
		char *local_ip;
};

struct config parse_args(int argc, char *argv[]) {
		struct config cfg = {0};
		if (argc != 4) {
				fprintf(stderr, "Usage: %s <peer_ip>:<peer_port> <local_ip>:<local_port>\n", argv[0]);
				exit(1);
		}
		char *peer_arg = argv[1];
		char *local_arg = argv[2];

		char *colon = strchr(peer_arg, ':');
		if (!colon) {
				fprintf(stderr, "Invalid peer argument format. Expected <peer_ip>:<peer_port>\n");
				exit(1);
		}
		*colon = '\0';
		cfg.peer_ip = peer_arg;
		cfg.peer_port = atoi(colon + 1);

		colon = strchr(local_arg, ':');
		if (!colon) {
				fprintf(stderr, "Invalid local argument format. Expected <local_ip>:<local_port>\n");
				exit(1);
		}
		*colon = '\0';
		cfg.local_ip = local_arg;
		cfg.local_port = atoi(colon + 1);

		return cfg;
}


int main(int argc, char *argv[]) {
		// Example usage: `sudo ./tunnel <peer_ip>:<peer_port> <local_ip>:<local_port>`
		struct config cfg = parse_args(argc, argv);
		int tun_fd = tun_open("tun0");
		if (tun_fd < 0) {
				return 1;
		}

		int udp_fd = udp_open(cfg.local_port);
		if (udp_fd < 0) {
				close(tun_fd);
				return 1;
		}	

		printf("TUN interface 'tun0' created. Waiting for packets...\n");

		struct sockaddr_in peer = {
				.sin_family = AF_INET,
				.sin_port = htons(cfg.peer_port)
		};
		inet_pton(AF_INET, cfg.peer_ip, &peer.sin_addr);

		// ip addr add 10.0.0.1/24 dev tun0
		// ip link set tun0 mtu 1380 up
		tunnel(tun_fd, udp_fd, &peer);
}
