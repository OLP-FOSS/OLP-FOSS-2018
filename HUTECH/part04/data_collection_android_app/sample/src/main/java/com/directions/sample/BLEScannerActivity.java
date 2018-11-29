package com.directions.sample;

import android.Manifest;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.google.android.gms.appindexing.Action;
import com.google.android.gms.appindexing.AppIndex;
import com.google.android.gms.common.api.GoogleApiClient;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class BLEScannerActivity extends AppCompatActivity {

    BluetoothManager btManager;
    BluetoothAdapter btAdapter;
    BluetoothLeScanner btScanner;
    Button startScanningButton;
    Button stopScanningButton;
    Button writeTestButton;
    Button testButton;
    ListView listview;
    TextView peripheralTextView;
    private final static int REQUEST_ENABLE_BT = 1;
    private static final int PERMISSION_REQUEST_COARSE_LOCATION = 1;

    Boolean btScanning = false;
    int deviceIndex = 0;
    ArrayList<BluetoothDevice> devicesDiscovered = new ArrayList<BluetoothDevice>();
    EditText deviceIndexInput;
    Button connectToDevice;
    Button disconnectDevice;
    BluetoothGatt bluetoothGatt;

    public final static String ACTION_GATT_CONNECTED =
            "com.example.bluetooth.le.ACTION_GATT_CONNECTED";
    public final static String ACTION_GATT_DISCONNECTED =
            "com.example.bluetooth.le.ACTION_GATT_DISCONNECTED";
    public final static String ACTION_GATT_SERVICES_DISCOVERED =
            "com.example.bluetooth.le.ACTION_GATT_SERVICES_DISCOVERED";
    public final static String ACTION_DATA_AVAILABLE =
            "com.example.bluetooth.le.ACTION_DATA_AVAILABLE";
    public final static String EXTRA_DATA =
            "com.example.bluetooth.le.EXTRA_DATA";

    public Map<String, String> uuids = new HashMap<String, String>();

    // Stops scanning after 5 seconds.
    private Handler mHandler = new Handler();
    private static final long SCAN_PERIOD = 15000;
    /**
     * ATTENTION: This was auto-generated to implement the App Indexing API.
     * See https://g.co/AppIndexing/AndroidStudio for more information.
     */
//    private GoogleApiClient client;
    private BleAdapter customAdapter;


    private void changeStatusBarColor(String color){
        if (Build.VERSION.SDK_INT >= 21) {
            Window window = getWindow();
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            window.setStatusBarColor(Color.parseColor(color));
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ble_scan);

        getSupportActionBar().hide();
        changeStatusBarColor("#002297");

        listview = (ListView) findViewById(R.id.listview);
        customAdapter = new BleAdapter(this, R.layout.item_ble, new ArrayList<BluetoothDevice>());
        listview.setAdapter(customAdapter);
        listview.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                Toast.makeText(BLEScannerActivity.this, "Connecting...", Toast.LENGTH_SHORT).show();

                bluetoothGatt = devicesDiscovered.get(i).connectGatt(BLEScannerActivity.this, false, btleGattCallback);
                BLEManager.getInstance().setBluetoothGatt(bluetoothGatt);
            }
        });


//        peripheralTextView = (TextView) findViewById(R.id.PeripheralTextView);
//        peripheralTextView.setMovementMethod(new ScrollingMovementMethod());
//        deviceIndexInput = (EditText) findViewById(R.id.InputIndex);
//        deviceIndexInput.setText("0");

//        connectToDevice = (Button) findViewById(R.id.ConnectButton);
//        connectToDevice.setOnClickListener(new View.OnClickListener() {
//            public void onClick(View v) {
//                connectToDeviceSelected();
//            }
//        });

        testButton = (Button) findViewById(R.id.btn_test_drop);
        testButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Intent intent = new Intent(BLEScannerActivity.this, MLActivity.class);
                startActivity(intent);
            }
        });

        writeTestButton = (Button) findViewById(R.id.writeTest);
        writeTestButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                BLEManager.getInstance().writeCharacterristic("bambi_test_thu_ne");
            }
        });

        disconnectDevice = (Button) findViewById(R.id.DisconnectButton);
        disconnectDevice.setVisibility(View.INVISIBLE);
        disconnectDevice.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                disconnectDeviceSelected();
            }
        });

        startScanningButton = (Button) findViewById(R.id.StartScanButton);
        startScanningButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                startScanning();
            }
        });

        stopScanningButton = (Button) findViewById(R.id.StopScanButton);
        stopScanningButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                stopScanning();
            }
        });
        stopScanningButton.setVisibility(View.INVISIBLE);

        btManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        btAdapter = btManager.getAdapter();
        BLEManager.getInstance().setBluetoothAdapter(btAdapter);
        btScanner = btAdapter.getBluetoothLeScanner();

        if (btAdapter != null && !btAdapter.isEnabled()) {
            Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
        }

        // Make sure we have access coarse location enabled, if not, prompt the user to enable it
        if (this.checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            final AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("This app needs location access");
            builder.setMessage("Please grant location access so this app can detect peripherals.");
            builder.setPositiveButton(android.R.string.ok, null);
            builder.setOnDismissListener(new DialogInterface.OnDismissListener() {
                @Override
                public void onDismiss(DialogInterface dialog) {
                    requestPermissions(new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, PERMISSION_REQUEST_COARSE_LOCATION);
                }
            });
            builder.show();
        }

//        client = new GoogleApiClient.Builder(this).addApi(AppIndex.API).build();
    }

    // Device scan callback.
    private ScanCallback leScanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            boolean isExist = false;
            for(int i = 0 ; i < devicesDiscovered.size() ; i++) {
                if(!TextUtils.isEmpty(devicesDiscovered.get(i).getAddress()) && !TextUtils.isEmpty(result.getDevice().getAddress())
                        && devicesDiscovered.get(i).getAddress().equals(result.getDevice().getAddress())) {
                    isExist = true;
                }
            }

            if(!isExist) {
                devicesDiscovered.add(result.getDevice());
                deviceIndex++;
                customAdapter.setData(devicesDiscovered);
            }
        }
    };

    // Device connect call back
    private final BluetoothGattCallback btleGattCallback = new BluetoothGattCallback() {

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, final BluetoothGattCharacteristic characteristic) {
            // this will get called anytime you perform a read or write characteristic operation
            BLEScannerActivity.this.runOnUiThread(new Runnable() {
                public void run() {
                    Toast.makeText(BLEScannerActivity.this, "Device read or wrote successfully", Toast.LENGTH_SHORT).show();
                }
            });
        }

        @Override
        public void onConnectionStateChange(final BluetoothGatt gatt, final int status, final int newState) {
            // this will get called when a device connects or disconnects
            System.out.println(newState);
            switch (newState) {
                case 0:
                    BLEScannerActivity.this.runOnUiThread(new Runnable() {
                        public void run() {
                            Toast.makeText(BLEScannerActivity.this, "Device disconnected", Toast.LENGTH_SHORT).show();
//                            connectToDevice.setVisibility(View.VISIBLE);
                            disconnectDevice.setVisibility(View.INVISIBLE);
                        }
                    });
                    break;
                case 2:
                    BLEScannerActivity.this.runOnUiThread(new Runnable() {
                        public void run() {
                            Toast.makeText(BLEScannerActivity.this, "Device connected", Toast.LENGTH_SHORT).show();

//                            connectToDevice.setVisibility(View.INVISIBLE);
                            disconnectDevice.setVisibility(View.VISIBLE);

                            Intent intent = new Intent(BLEScannerActivity.this, MainActivity.class);
                            startActivity(intent);
                        }
                    });

                    // discover services and characteristics for this device
                    bluetoothGatt.discoverServices();

                    break;
                default:
                    BLEScannerActivity.this.runOnUiThread(new Runnable() {
                        public void run() {
                        Toast.makeText(BLEScannerActivity.this, "We encounterned an unknown state", Toast.LENGTH_SHORT).show();
                        }
                    });
                    break;
            }
        }

        @Override
        public void onServicesDiscovered(final BluetoothGatt gatt, final int status) {
//            displayGattServices(bluetoothGatt.getServices());
        }

        @Override
        // Result of a characteristic read operation
        public void onCharacteristicRead(BluetoothGatt gatt,
                                         BluetoothGattCharacteristic characteristic,
                                         int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);
            }
        }
    };

    private void broadcastUpdate(final String action,
                                 final BluetoothGattCharacteristic characteristic) {

        System.out.println(characteristic.getUuid());
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        switch (requestCode) {
            case PERMISSION_REQUEST_COARSE_LOCATION: {
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    System.out.println("coarse location permission granted");
                } else {
                    final AlertDialog.Builder builder = new AlertDialog.Builder(this);
                    builder.setTitle("Functionality limited");
                    builder.setMessage("Since location access has not been granted, this app will not be able to discover beacons when in the background.");
                    builder.setPositiveButton(android.R.string.ok, null);
                    builder.setOnDismissListener(new DialogInterface.OnDismissListener() {

                        @Override
                        public void onDismiss(DialogInterface dialog) {
                        }

                    });
                    builder.show();
                }
                return;
            }
        }
    }

    public void startScanning() {
        System.out.println("start scanning");
        btScanning = true;
        deviceIndex = 0;
        devicesDiscovered.clear();
        Toast.makeText(BLEScannerActivity.this, "Started Scanning", Toast.LENGTH_SHORT).show();
        startScanningButton.setVisibility(View.INVISIBLE);
        stopScanningButton.setVisibility(View.VISIBLE);
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                btScanner.startScan(leScanCallback);
            }
        });
        customAdapter.setData(devicesDiscovered);

        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                stopScanning();
            }
        }, SCAN_PERIOD);
    }

    public void stopScanning() {
        System.out.println("stopping scanning");
        Toast.makeText(BLEScannerActivity.this, "Stopped Scanning", Toast.LENGTH_SHORT).show();
        btScanning = false;
        startScanningButton.setVisibility(View.VISIBLE);
        stopScanningButton.setVisibility(View.INVISIBLE);
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                btScanner.stopScan(leScanCallback);
            }
        });

        customAdapter.setData(devicesDiscovered);
    }

//    public void connectToDeviceSelected() {
//        peripheralTextView.append("Trying to connect to device at index: " + deviceIndexInput.getText() + "\n");
//        int deviceSelected = Integer.parseInt(deviceIndexInput.getText().toString());
//        bluetoothGatt = devicesDiscovered.get(deviceSelected).connectGatt(this, false, btleGattCallback);
//    }

    public void disconnectDeviceSelected() {
        Toast.makeText(BLEScannerActivity.this, "Disconnecting from the device", Toast.LENGTH_SHORT).show();
        bluetoothGatt.disconnect();
    }
//
//    private void displayGattServices(List<BluetoothGattService> gattServices) {
//        if (gattServices == null) return;
//
//        // Loops through available GATT Services.
//        for (BluetoothGattService gattService : gattServices) {
//
//            final String uuid = gattService.getUuid().toString();
//            new ArrayList<HashMap<String, String>>();
//            List<BluetoothGattCharacteristic> gattCharacteristics =
//                    gattService.getCharacteristics();
//
//            // Loops through available Characteristics.
//            for (BluetoothGattCharacteristic gattCharacteristic :
//                    gattCharacteristics) {
//
//                final String charUuid = gattCharacteristic.getUuid().toString();
//                System.out.println("Characteristic discovered for service: " + charUuid);
//                BLEScannerActivity.this.runOnUiThread(new Runnable() {
//                    public void run() {
//                        peripheralTextView.append("Characteristic discovered for service: "+charUuid+"\n");
//                    }
//                });
//            }
//        }
//    }

    @Override
    public void onStart() {
        super.onStart();

//        client.connect();
//        Action viewAction = Action.newAction(
//                Action.TYPE_VIEW, // TODO: choose an action type.
//                "Main Page", // TODO: Define a title for the content shown.
//                // TODO: If you have web page content that matches this app activity's content,
//                // make sure this auto-generated web page URL is correct.
//                // Otherwise, set the URL to null.
//                Uri.parse("http://host/path"),
//                // TODO: Make sure this auto-generated app URL is correct.
//                Uri.parse("android-app://com.example.joelwasserman.androidbleconnectexample/http/host/path")
//        );
//        AppIndex.AppIndexApi.start(client, viewAction);
    }

    @Override
    public void onStop() {
        super.onStop();

//        Action viewAction = Action.newAction(
//                Action.TYPE_VIEW, // TODO: choose an action type.
//                "Main Page", // TODO: Define a title for the content shown.
//                // TODO: If you have web page content that matches this app activity's content,
//                // make sure this auto-generated web page URL is correct.
//                // Otherwise, set the URL to null.
//                Uri.parse("http://host/path"),
//                // TODO: Make sure this auto-generated app URL is correct.
//                Uri.parse("android-app://com.example.joelwasserman.androidbleconnectexample/http/host/path")
//        );
//        AppIndex.AppIndexApi.end(client, viewAction);
//        client.disconnect();
    }
}
