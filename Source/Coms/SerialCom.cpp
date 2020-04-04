#include "SerialCom.h"
#include "Core/Logging.h"
#include "Graphics/UI/IMGUI/imgui.h"

#include <Windows.h>

static DWORD ToNativeBaudRate(SerialCom::BaudRate::T br)
{
	switch (br)
	{
	case SerialCom::BaudRate::BR_4800:
		return CBR_4800;
	case SerialCom::BaudRate::BR_9600:
		return CBR_9600;
	case SerialCom::BaudRate::BR_14400:
		return CBR_14400;
	case SerialCom::BaudRate::COUNT:
	default:
		return 0;
	}
}

SerialCom::SerialCom()
	:mActivePort("NULL")
	,mPortHandle(INVALID_HANDLE_VALUE)
	,mBaudRate(BaudRate::BR_9600)
{
}

void SerialCom::RenderUI()
{
	ImGui::Begin("Coms");
	{
		if (ImGui::Button("Refresh Ports"))
		{
			mCurPorts = GetSerialPorts();
			INFO("Found:%i ports", mCurPorts.size());
		}
		
		// Select port:
		if (ImGui::BeginCombo("Ports", mActivePort.c_str()))
		{
			for (const auto p : mCurPorts)
			{
				bool selected = false;
				if(mActivePort == p)
				{
					selected = true;
				}
				if (ImGui::Selectable(p.c_str(), selected))
				{
					if(!selected)
					{
						Connect(p);
					}
				}
			}
			ImGui::EndCombo();
		}

		// Select BR:
		if (ImGui::BeginCombo("Baud Rate", BaudRate::ToStr(mBaudRate)))
		{
			for (int br = 0; br < BaudRate::COUNT; ++br)
			{
				BaudRate::T cur = (BaudRate::T)br;
				bool selected = cur == mBaudRate;
				if (ImGui::Selectable(BaudRate::ToStr(cur), selected))
				{
					if (!selected)
					{
						UpdateBaudRate(cur);
					}
				}
			}
			ImGui::EndCombo();
		}
	}
	ImGui::End();
}

bool SerialCom::IsConnected() const
{
	return mPortHandle != INVALID_HANDLE_VALUE;
}

void SerialCom::CloseConnection()
{
	if (IsConnected())
	{
		CloseHandle(mPortHandle);
		mPortHandle = INVALID_HANDLE_VALUE;
	}
}

void SerialCom::UpdateBaudRate(BaudRate::T newRate)
{
	mBaudRate = newRate; // Always cache it for the UI

	if (!IsConnected()) 
	{
		return;
	}

	DCB params = {};
	params.DCBlength = sizeof(params);
	if (GetCommState(mPortHandle, &params))
	{
		params.BaudRate = ToNativeBaudRate(mBaudRate);

		if (!SetCommState(mPortHandle, &params))
		{
			ERR("Could not set the coms state");
		}
	}
	else
	{
		ERR("Could not get coms state");
	}
}

void SerialCom::Connect(std::string port)
{
	CloseConnection();
	mPortHandle = CreateFileA(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (mPortHandle == INVALID_HANDLE_VALUE)
	{
		ERR("Failed to connect to:%s", port.c_str());
		return;
	}
	
	mActivePort = port;

	DCB params = {};
	params.DCBlength = sizeof(params);
	if (GetCommState(mPortHandle, &params))
	{
		params.BaudRate = ToNativeBaudRate(mBaudRate);
		if (!SetCommState(mPortHandle, &params))
		{
			ERR("Could not set the coms state");
		}

		COMMTIMEOUTS timeout = {};
		timeout.ReadIntervalTimeout = 2;
		timeout.ReadTotalTimeoutConstant = 2;
		timeout.ReadTotalTimeoutMultiplier = 0;
		timeout.WriteTotalTimeoutConstant = 0;
		timeout.WriteTotalTimeoutMultiplier = 0;
		if (!SetCommTimeouts(mPortHandle, &timeout))
		{
			ERR("Failed to set port timeouts");
		}
	}
	else
	{
		ERR("Could not get coms state");
	}	
}

int SerialCom::ReadBytes(char * buffer, int size)
{
	if (!IsConnected())
	{
		return 0;
	}
	DWORD  numBytesRead = 0;
	ReadFile(mPortHandle, buffer, size, &numBytesRead, NULL);
	return (int)numBytesRead;
}

std::vector<std::string> SerialCom::GetSerialPorts()
{
	std::vector<std::string> portsToUse;
	char portName[16];
	HANDLE port = {};
	int systemPorts = 9; // COM1-COM9
	for (int pIdx = 0; pIdx < systemPorts; ++pIdx)
	{
		sprintf_s(portName, 16, "COM%i", pIdx + 1);
		port = CreateFileA(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (port == INVALID_HANDLE_VALUE)
		{
			DWORD res = GetLastError();
		}
		else
		{
			portsToUse.push_back(portName);
			CloseHandle(port);
		}
	}
	return portsToUse;
}
