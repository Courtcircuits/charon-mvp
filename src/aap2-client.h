#include "proto/aap2.pb-c.h"

struct aap2_client {
  int socket_fd;
  char *node_id;
};

struct aap2_client* connect_aap2(const char *path);
int configure_aap2(struct aap2_client *client, int is_subscriber, Aap2__AuthType auth_type, const char *secret, const char *endpoint_id);

int send_aap2(struct aap2_client *client, const char *dst_eid, const uint8_t *payload, size_t payload_len);
int close_aap2(struct aap2_client *client);
int listen_aap2(struct aap2_client *client);
