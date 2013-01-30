package com.parrot.arsdk.libnetwork;

/**
 *  Class used to store the deported data received
**/
public class NetworkDataRecv 
{
    private byte[] m_data;
    private int m_limitSize;
    private int m_readSize;
    
    /**
     *  Constructor
     *  @param[in] limitSize maximum size of the data read
    **/
    public NetworkDataRecv(int limitSize) 
    {
        m_data = new byte[limitSize];
        m_limitSize = limitSize;
        m_readSize = 0;
    }
    
    /**
     *  Get data 
     *  @return byte array of the data read
    **/
    public byte[] getData()
    {
        return m_data;
    }
    
    /**
     *  Get size of data read
     *  @return size of the data read
    **/
    public int getReadSize() 
    {
		return m_readSize;
	}
    
    /**
     *  Convert the data read to string
     *  @return string from the data read
    **/
    public String toString()
    {
        String ret = "{";
        if( null != m_data)
        {
            int minSize = Math.min (Math.min (m_limitSize, m_readSize), m_data.length);
            for (int i = 0; i < minSize; i++)
            {
                ret += m_data[i] + ", ";
            }
        }
        return ret + "}";
    }

}
