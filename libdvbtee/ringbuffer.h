class RingBuffer {
public:
#if 0
    RingBuffer(int) {}
#else
    RingBuffer() {}
    void setCapacity(int) {}
#endif
    virtual ~RingBuffer() {}
    int getSize() const { return 0; }
    int getCapacity() const { return 0; }
    void reset() {}
    bool write(const void*, int) { return false; }
    int read(void*, int) { return 0; }
};
