/**
 * @file RingQueue.h
 *
 * Declaration of class RingQueue
 *
 * @author Max Risler
 */

#ifndef __RingQueue_h_
#define __RingQueue_h_

/**
 * @class RingQueue
 *
 * template class for cyclic buffering of the last n values of Type V
 */
template <class V, int n> class RingQueue
{
  public:
    /** Constructor */
    RingQueue() {init();}

    /**
     * initializes the Ringbuffer
     */
    void init () {frontIndex = 0; backIndex = 0; numberOfEntries = 0;}

    /**
     * adds an entry to the buffer
     * \param v value to be added
     */
    void push (const V& v) 
    {
      if (numberOfEntries == n){
        numberOfEntries = n-1;
        incrementIndex(frontIndex);
      }
      buffer[backIndex] = v;
      incrementIndex(backIndex);
      numberOfEntries++;
    }

    void incrementIndex(int &index){
      index++;
      if (index==n) index = 0;
    }

    const V& pop () 
    {
      if (numberOfEntries <= 0) return buffer[0];
      int temp = frontIndex;
      incrementIndex(frontIndex);
      numberOfEntries--;
      return buffer[temp];
    }

    const V& front() const
    {
      return buffer[frontIndex];
    }

    const V& back() const
    {
      return buffer[backIndex-1];
    }

    const V& getEntry (int i) const
    {
      int index = frontIndex + i;
      while (index >= n) index -= n;
      return buffer[index];
    }

    V& getEntry(int i) {
      int index = frontIndex + i;
      while (index >= n) index -= n;
      return buffer[index];
    }

    /**
     * returns an entry
     * \param i index of entry counting from last added (last=0,...)
     * \return a reference to the buffer entry
     */
    const V& operator[] (int i) const
    {
      return getEntry(i);
    }

    V& operator[] (int i) {
      return getEntry(i);
    }

    /** Returns the number of elements that are currently in the ring buffer
    * \return The number
    */
    int getNumberOfEntries() const
    {
      return numberOfEntries;
    }

    /**
    * Returns the maximum entry count.
    * \return The maximum entry count.
    */
    inline int getMaxEntries() const
    {
      return n;
    }

  private:
    int frontIndex;
    int backIndex;
    int numberOfEntries;
    V buffer[n];
};

#endif // __RingQueue_h_
