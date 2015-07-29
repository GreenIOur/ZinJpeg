
// initializes concealment subsystem
void concealment_initialize();
// notifies the concealment subsystem that a picture has been written correctly
void concealment_block_written_success(int blknr, short* block);

// request to the concealment subsys to write <count> number of
// concealed blocks, the first concealed block has id <blknr>
void concealment_write_concealment_blocks(int blknr, int count);
// shuts down the concealment engine
void concealment_finalize();
