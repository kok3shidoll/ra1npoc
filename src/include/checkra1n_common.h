
#ifdef HAVE_DEBUG
#define DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif

int payload_stage2(io_client_t client, uint16_t cpid, checkra1n_payload_t payload);
int pongo(io_client_t client, uint16_t cpid, checkra1n_payload_t payload);
int connect_to_stage2(io_client_t client, uint16_t cpid, checkra1n_payload_t payload);
