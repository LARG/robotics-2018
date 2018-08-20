/**
 * @file RingQueueWithSum.h
 *
 * Declaration of class RingQueueWithSum
 *
 * @author Max Risler
 */

#ifndef __RingQueueWithSum_h_
#define __RingQueueWithSum_h_

/**
 * @class RingQueueWithSum
 *
 * template class for cyclic buffering of the last n values of Type V
 */
template <class V, int n> class RingQueueWithSum
{
  public:
    /** Constructor */
    RingQueueWithSum() {init();}

    /**
     * initializes the Ringbuffer
     */
    void init () {frontIndex = 0; backIndex = 0; numberOfEntries = 0; sum = V();}

    /**
     * adds an entry to the buffer
     * \param v value to be added
     */
    void push (const V& v) 
    {
      if (numberOfEntries == n){
        numberOfEntries = n-1;
        sum -= buffer[frontIndex];
        incrementIndex(frontIndex);
      }
      sum += v;
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
      sum -= buffer[temp];
      incrementIndex(frontIndex);
      numberOfEntries--;
      return buffer[temp];
    }

    const V& front() const
    {
      return buffer[frontIndex];
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

    V getSum() const {return sum;}
    V getAverage() const {
      if (numberOfEntries == 0) {
        return V();
      }
      return sum / numberOfEntries;
    }

  private:
    int frontIndex;
    int backIndex;
    int numberOfEntries;
    V buffer[n];
    V sum;
};

#endif // __RingQueueWithSum_h_
