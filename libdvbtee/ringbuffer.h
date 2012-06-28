class RingBuffer {
public:
    RingBuffer() {}
    virtual ~RingBuffer() {}
    void set_capacity(int) {}
    int get_size() const { return 0; }
    int get_capacity() const { return 0; }
    void reset() {}
    bool write(const void*, int) { return false; }
    int read(void*, int) { return 0; }
    int read_ptr(void*, int) { return 0; }
    void advance_read_ptr() {}
};
