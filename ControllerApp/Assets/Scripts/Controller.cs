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

    private string ConnectedToAdr = "NULL";
    public string CommandsServiceUID = "1101";
    public string ThrottleCharacteristicUID = "2202";
    public string YawCharacteristicUID = "2203";
    public string PitchleCharacteristicUID = "2204";
    public string RollleCharacteristicUID = "2205";

    public Joystick LeftJoystick;
    public Joystick RightJoystick;

    // Timer to delay sending commands.
    private float ConnectTimer = 0.0f;
    private float SendTimer = 999.0f;
    // Time between each set of BT writes.
    public float SendCommandsDelay = 0.2f;

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
                float throttle = Mathf.Max(LeftJoystick.Vertical * 100.0f, 0.0f);
                float yaw = LeftJoystick.Horizontal * 100.0f;
                float pitch = RightJoystick.Vertical * 100.0f;
                float roll = RightJoystick.Horizontal * 100.0f;

                SendTimer += Time.deltaTime;
                if(SendTimer > SendCommandsDelay)
                {
                    SendTimer = 0.0f;
                    SendInt((int)throttle, CommandsServiceUID, ThrottleCharacteristicUID);
                    SendInt((int)yaw, CommandsServiceUID, YawCharacteristicUID);
                    SendInt((int)pitch, CommandsServiceUID, PitchleCharacteristicUID);
                    SendInt((int)roll, CommandsServiceUID, RollleCharacteristicUID);
                }
            }
        }
        else
        {
        }
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
                // Reset state, go back to Scan:
                ConnectTimer = 0.0f;
                ConnectedToAdr = "NULL";
                ResetPeripherals();
                UpdateState(States.Scan);
            });
        }
    }

    string FullUUID (string uuid)
	{
		return "0000" + uuid + "-0000-1000-8000-00805F9B34FB";
	}

    private void SendInt(int value, string serviceUID, string characteristicUUID)
    {
        string fullService = FullUUID(serviceUID);
        string fullCharacteristic = FullUUID(characteristicUUID);
        byte[] data = BitConverter.GetBytes(value);
		BluetoothLEHardwareInterface.WriteCharacteristic (ConnectedToAdr, fullService, fullCharacteristic, data, data.Length, false, null);
    }

    private void UpdateState(States newState)
    {
        if(newState == States.Scan)
        {
            LeftJoystick.gameObject.SetActive(false);
            RightJoystick.gameObject.SetActive(false);

            DropdownUI.gameObject.SetActive(true);
            ConnectButtonUI.gameObject.SetActive(true);
        }
        else if(newState == States.Connected)
        {
            LeftJoystick.gameObject.SetActive(true);
            RightJoystick.gameObject.SetActive(true);
            
            DropdownUI.gameObject.SetActive(false);
            ConnectButtonUI.gameObject.SetActive(false);
        }
        CurState = newState;
    }
}
