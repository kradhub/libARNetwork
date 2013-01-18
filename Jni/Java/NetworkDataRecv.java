package com.parrot.arsdk.libnetwork;

public class NetworkDataRecv 
{
    private byte[] m_data;
    private int m_limitSize;
    private int m_readSize;
    
    public NetworkDataRecv(int limitSize) 
    {
        m_data = new byte[limitSize];
        m_limitSize = limitSize;
        m_readSize = 0;
    }
    
    public byte[] getData()
    {
        return m_data;
    }
    
    public int getReadSize() 
    {
		return m_readSize;
	}
    
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
