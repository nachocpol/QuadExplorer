using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class Controller : MonoBehaviour
{
    public enum States
    {
        Startup,
        Scan,
        Scanning,
        Connected,
        Error,
    }
    private States CurState = States.Startup;
    public string DeviceName = "QuadExplorer";

    // Map of peripherals. Key is the address.
    public Dictionary<string,string> DiscoveredPeripherals; 
    private int SelectedPeripheralIdx = 0;
    public Dropdown DropdownUI;
    public Button ConnectButtonUI;
    public Button StopButtonUI;
    public Joystick LeftJoystick;
    public Joystick RightJoystick;
    public Slider RollTrimUI;
    public Slider PitchTrimUI;

    private string ConnectedToAdr = "NULL";
    public string CommandsServiceUID = "1101";
    public string ThrottleCharacteristicUID = "2202";
    public string YawCharacteristicUID = "2203";
    public string PitchleCharacteristicUID = "2204";
    public string RollleCharacteristicUID = "2205";
    public string StopCharacteristicUID = "2206";
    public string PackedCharacteristicUID = "2207";

    // Timer to delay sending commands.
    private float ConnectTimer = 0.0f;
    private float SendTimer = 999.0f;
    // Time between each set of BT writes.
    public float SendCommandsDelay = 0.2f;

    private static bool PendingResponse = false;

    void Start()
    {
        DiscoveredPeripherals = new Dictionary<string, string>();        
        Input.location.Start();

        if(!Application.isEditor)
        {
            bool initAsCentral = true;
            bool initAsPeripheral = false;
            BluetoothLEHardwareInterface.Initialize (initAsCentral, initAsPeripheral, 
            () => 
            {
                UpdateState(States.Scan);
                BluetoothLEHardwareInterface.BluetoothScanMode(BluetoothLEHardwareInterface.ScanMode.LowLatency);
                BluetoothLEHardwareInterface.BluetoothConnectionPriority(BluetoothLEHardwareInterface.ConnectionPriority.High);
		    },
            (error) => 
            {
                Debug.Log(">>>>>>>>>>> [BLE ERROR] " + error);
                CurState = States.Error;
                HandleDisconnected();
		    });   
        }   
        else
        {
            UpdateState(States.Connected);
        } 
    }

    void Update()
    {
        if(CurState == States.Error)
        {
            return;
        }

        if(CurState == States.Scan)
        {
            UpdateState(States.Scanning);
            BluetoothLEHardwareInterface.ScanForPeripheralsWithServices (null, 
            (address, name) => 
            {    
                DiscoverPeripheral(name,address);
            }, 
            (address, name, rssi, bytes) => 
            {
                DiscoverPeripheral(name, address);
            }, false);
        }
        else if(CurState == States.Scanning)
        {
            // Just wait, the scan callbacks will keep finding new peripherals.
        }
        else if(CurState == States.Connected)
        {
            // We have a 2seconds time for the device to get ready:
            ConnectTimer += Time.deltaTime;
            if(ConnectTimer > 2.0f)
            {
                float throttle = Mathf.Max(LeftJoystick.Vertical * 255.0f, 0.0f);
                float yaw = LeftJoystick.Horizontal * 127.0f;
                float pitch = Mathf.Clamp((RightJoystick.Vertical * 127.0f) + PitchTrimUI.value, -127.0f, 127.0f);
                float roll = Mathf.Clamp((RightJoystick.Horizontal * 127.0f) + RollTrimUI.value, -127.0f, 127.0f);

                SendTimer += Time.deltaTime;
                if(SendTimer > SendCommandsDelay)
                {
                    SendTimer = 0.0f;
                    //SendValue((int)throttle, CommandsServiceUID, ThrottleCharacteristicUID);
                    //SendValue((int)yaw, CommandsServiceUID, YawCharacteristicUID);
                    //SendValue((int)pitch, CommandsServiceUID, PitchleCharacteristicUID);
                    //SendValue((int)roll, CommandsServiceUID, RollleCharacteristicUID);
                    UInt32 packedCommands = PackCommands((int)throttle, (int)yaw, (int)pitch, (int)roll);
                    SendValue(packedCommands,CommandsServiceUID, PackedCharacteristicUID);
                }
            }
        }
        else
        {
        }
    }

    UInt32 PackCommands(int throttle, int yaw, int pitch, int roll)
    {
        UInt32 result = 0;

        bool yawNegative = yaw < 0;
        bool pitchNegative = pitch < 0;
        bool rollNegative = roll < 0;

        yaw   = Mathf.Abs(yaw);
        pitch = Mathf.Abs(pitch);
        roll  = Mathf.Abs(roll);

        result |= (UInt32)((byte)throttle) << 0 ;
        result |= (UInt32)((byte)yaw)      << 8 ;
        result |= (UInt32)((byte)pitch)    << 16;
        result |= (UInt32)((byte)roll)     << 24;

        result |= (UInt32)(yawNegative   ? 1 : 0) << 15;
        result |= (UInt32)(pitchNegative ? 1 : 0) << 23;
        result |= (UInt32)(rollNegative  ? 1 : 0) << 31;

        return result;
    }

    private void DiscoverPeripheral(string name, string adr)
    {
        string val = "";
        if(!DiscoveredPeripherals.TryGetValue(adr, out val))
        {
            Debug.Log("New peripheral: " + name + "," + adr);
            DiscoveredPeripherals[adr] = name;

            // Add it to the UI:
            Dropdown.OptionData data = new Dropdown.OptionData(name + "," + adr);
            DropdownUI.options.Add(data);
            DropdownUI.RefreshShownValue();
        }
    }

    private void ResetPeripherals()
    {
        DiscoveredPeripherals.Clear();
        DropdownUI.options.Clear();
        DropdownUI.RefreshShownValue();
    }

    public void OnDropdownUIValueChange(int newIndex)
    {
        SelectedPeripheralIdx = newIndex;
    }

    public void OnConnectClicked()
    {
        if(CurState ==  States.Connected)
        {
            return;
        }

        var curOption = DropdownUI.options[SelectedPeripheralIdx];
        string[] data = curOption.text.Split(','); // name,addr
        ConnectedToAdr = data[1];
        string val = "";
        if(DiscoveredPeripherals.TryGetValue(ConnectedToAdr, out val))
        {
            BluetoothLEHardwareInterface.ConnectToPeripheral (ConnectedToAdr, 
            (name)=>
            {
                Debug.Log("Connected to: " + name);
                BluetoothLEHardwareInterface.StopScan();
                UpdateState(States.Connected);
            }, null, 
            (address, serviceUUID, characteristicUUID) => 
            {
                // This callback should be  giving us each service - char, but seems to only send the services.
            }, (disconnectedAddress) => 
            {
                HandleDisconnected();
            });
        }
    }

    private void HandleDisconnected()
    {
        if(ConnectedToAdr != "NULL")
        {
            BluetoothLEHardwareInterface.DisconnectPeripheral(ConnectedToAdr, null);
        }
        // Reset state, go back to Scan:
        PendingResponse = false;
        ConnectTimer = 0.0f;
        ConnectedToAdr = "NULL";
        ResetPeripherals();
        UpdateState(States.Scan);
    }

    public void OnStopClicked()
    {
        // Send it with response, as this is a critical message!
        string fullService = FullUUID(CommandsServiceUID);
        string fullCharacteristic = FullUUID(StopCharacteristicUID);
        byte[] one = new byte[1];
        one[0] = 0x1;
        BluetoothLEHardwareInterface.WriteCharacteristic(ConnectedToAdr, fullService, fullCharacteristic, one, 1, true, 
        (msg)=>
        {
            Debug.Log(msg);
            HandleDisconnected();
        });
    }

    string FullUUID (string uuid)
	{
		return "0000" + uuid + "-0000-1000-8000-00805F9B34FB";
	}

    private void SendValue(int value, string serviceUID, string characteristicUUID)
    {
        string fullService = FullUUID(serviceUID);
        string fullCharacteristic = FullUUID(characteristicUUID);
        byte[] data = BitConverter.GetBytes(value);
		BluetoothLEHardwareInterface.WriteCharacteristic (ConnectedToAdr, fullService, fullCharacteristic, data, data.Length, false, null);
    }

    private void SendValue(UInt32 value, string serviceUID, string characteristicUUID)
    {
        if(!PendingResponse)
        {
            PendingResponse = true;
            string fullService = FullUUID(serviceUID);
            string fullCharacteristic = FullUUID(characteristicUUID);
            byte[] data = BitConverter.GetBytes(value);
		    BluetoothLEHardwareInterface.WriteCharacteristic (ConnectedToAdr, fullService, fullCharacteristic, data, data.Length, true, 
            (msg)=>
            {
                Debug.Log(msg);
                PendingResponse = false;
            });
        }
    }

    private void UpdateState(States newState)
    {
        if(newState == States.Scan)
        {
            LeftJoystick.gameObject.SetActive(false);
            RightJoystick.gameObject.SetActive(false);
            StopButtonUI.gameObject.SetActive(false);

            DropdownUI.gameObject.SetActive(true);
            ConnectButtonUI.gameObject.SetActive(true);
        }
        else if(newState == States.Connected)
        {
            LeftJoystick.gameObject.SetActive(true);
            RightJoystick.gameObject.SetActive(true);
            StopButtonUI.gameObject.SetActive(true);

            DropdownUI.gameObject.SetActive(false);
            ConnectButtonUI.gameObject.SetActive(false);
        }
        CurState = newState;
    }

    private void OnApplicationPause (bool paused)
    {
        // Atm we can't allow a pause, so just disconnect:
        if(paused)
        {
            HandleDisconnected();
        }
    }
}
