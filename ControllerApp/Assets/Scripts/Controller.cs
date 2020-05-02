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

    private string ConnectedToAdr = "NULL";
    public string CommandsServiceUID = "1101";
    public string ThrottleCharacteristicUID = "2202";

    // Timer to delay sending commands.
    private float ConnectTimer = 0.0f;

    void Start()
    {
        DiscoveredPeripherals = new Dictionary<string, string>();

        Input.location.Start();

        bool initAsCentral = true;
        bool initAsPeripheral = false;
        BluetoothLEHardwareInterface.Initialize (initAsCentral, initAsPeripheral, 
        () => 
        {
            CurState = States.Scan;
            BluetoothLEHardwareInterface.BluetoothScanMode(BluetoothLEHardwareInterface.ScanMode.LowLatency);
            BluetoothLEHardwareInterface.BluetoothConnectionPriority(BluetoothLEHardwareInterface.ConnectionPriority.High);
		},
        (error) => 
        {
            Debug.Log(">>>>>>>>>>> [BLE ERROR] " + error);
            CurState = States.Error;
		});       
    }

    void Update()
    {
        if(CurState == States.Error)
        {
            return;
        }

        if(CurState == States.Scan)
        {
            Debug.Log(">>>>>>>>>>>>>>> Started scan <<<<<<<<<<<<<<<");   
            CurState = States.Scanning;
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
                // We are connected!
                int t = (int)(123.0f * (Mathf.Sin(Time.time * 0.5f) * 0.5f + 0.5f));
                SendInt(t, CommandsServiceUID, ThrottleCharacteristicUID);
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

    public void OnDropdownUIValueChange(int newIndex)
    {
        SelectedPeripheralIdx = newIndex;
    }

    public void OnConnectClicked()
    {
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
                CurState = States.Connected;
            }, null, 
            (address, serviceUUID, characteristicUUID) => 
            {
                // This callback should be  giving us each service - char, but seems to only send the services.
                // Debug.Log(">>>>>>>>> " + serviceUUID + ","   + characteristicUUID);
            }, (disconnectedAddress) => 
            {
                // Handle diconnection ! ! ! 
            });
        }
    }

    string FullUUID (string uuid)
	{
		return "0000" + uuid + "-0000-1000-8000-00805F9B34FB";
	}

    void SendInt(int value, string serviceUID, string characteristicUUID)
    {
        string fullService = FullUUID(serviceUID);
        string fullCharacteristic = FullUUID(characteristicUUID);
        //byte[] data = BitConverter.GetBytes(value);
        byte[] test = new byte[1];
        test[0] = 32;
		BluetoothLEHardwareInterface.WriteCharacteristic (ConnectedToAdr, fullService, fullCharacteristic, test, 1, false, 
        (charUID)=>
        {
            Debug.Log(">>>>>>>>>>>>>>>>>>>>>" + charUID);
        });
    }
}
