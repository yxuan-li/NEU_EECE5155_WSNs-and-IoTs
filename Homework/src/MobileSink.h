// position
double x;
double y;
double cosc;
double sinc;
double velocity;
double v_cos;
double v_sin;
double d;
double receive_distance;
double discovery_range;
double communication_range;


double on_time;

// sink variables
bool discoveried = false;

// wireless timer
double last_end_time = 0;

// sensor states
bool sensor_on;
bool waiting_LRB;
bool waiting_SRB;
bool waiting_beacon;
bool transmitting;

// sensor variables
double off_time;
double low_off_time;
double high_off_time;
int ack_lost = 0;
int packets_send = 0;
int discovery_time = 0;
double discovery_ratio=0;
double LRB_timeout_time;
bool is_LRB_timeout;

// energy
double p_rx;
double energy_discovery;
double p_tx;
double energy_transfer;
double P_duration;
double tmp_time;
double all_energy_discovery;
double all_energy_transfer;
double average_energy_discovery;
double average_energy_transfer;

// public
int passage_counter;
double throughput;


