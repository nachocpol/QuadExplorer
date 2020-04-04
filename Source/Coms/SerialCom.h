#pragma once

#include <vector>
#include <string>

typedef void* PortHandle;

class SerialCom
{
public:
	struct BaudRate
	{
		enum T
		{
			BR_4800,
			BR_9600,
			BR_14400,
			COUNT
		};
		static const char* ToStr(T& t)
		{
			switch (t)
			{
			case BR_4800:	return "4800";
			case BR_9600:	return "9600";
			case BR_14400:  return "14400";
			default:		return "Invalid";
			}
		}
	};

	SerialCom();
	void RenderUI();
	
	bool IsConnected()const;
	void CloseConnection();
	void UpdateBaudRate(BaudRate::T newRate);
	void Connect(std::string port);
	int ReadBytes(char* buffer, int size);

	static std::vector<std::string> GetSerialPorts();

private:
	std::vector<std::string> mCurPorts;
	std::string mActivePort;
	PortHandle mPortHandle;
	BaudRate::T mBaudRate;
};