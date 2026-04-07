#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/select.h>

#define TUN_DEVICE "/dev/net/tun"
#define MTU 1380



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


int main() {
		int tun_fd = tun_open("tun0");
		if (tun_fd < 0) {
				return 1;
		}

		printf("TUN interface 'tun0' created. Waiting for packets...\n");

		while (1) {
				char buffer[MTU];
				fd_set read_fds;
				FD_ZERO(&read_fds);
				FD_SET(tun_fd, &read_fds);

				int ret = select(tun_fd + 1, &read_fds, NULL, NULL, NULL);
				if (ret < 0) {
						perror("select");
						break;
				}

				if (FD_ISSET(tun_fd, &read_fds)) {
						ssize_t nread = read(tun_fd, buffer, sizeof(buffer));
						if (nread < 0) {
								perror("read");
								break;
						}
						printf("Read %zd bytes from tun0\n", nread);
						// Process the packet in 'buffer' as needed
				}
		}

		close(tun_fd);
		return 0;
}
