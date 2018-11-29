package com.directions.sample;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.util.Log;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.List;

/**
 * Created by macbook on 11/12/18.
 */

public class BLEManager {
    private static BLEManager instance;
    public static BLEManager getInstance() {
        if(instance == null) {
            instance = new BLEManager();
        }
        return instance;
    }

    private BluetoothGatt bluetoothGatt;
    private BluetoothAdapter bluetoothAdapter;

    public void setBluetoothGatt(BluetoothGatt bluetoothGatt) {
        this.bluetoothGatt = bluetoothGatt;
    }

    public BluetoothAdapter getBluetoothAdapter() {
        return bluetoothAdapter;
    }
    public void setBluetoothAdapter(BluetoothAdapter bluetoothAdapter) {
        this.bluetoothAdapter = bluetoothAdapter;
    }

    public BluetoothGatt getBluetoothGatt() {
        return bluetoothGatt;
    }

    public void writeCharacterristic(String data) {
        if(bluetoothGatt == null) {
            return;
        }

        List<BluetoothGattService> gattServices = bluetoothGatt.getServices();
        if (gattServices == null) return;

        // Loops through available GATT Services.
        for (BluetoothGattService gattService : gattServices) {
            List<BluetoothGattCharacteristic> gattCharacteristics =
                    gattService.getCharacteristics();

            // Loops through available Characteristics.
            for (BluetoothGattCharacteristic gattCharacteristic :
                    gattCharacteristics) {
                writeCharacteristic(gattCharacteristic, data);
            }
        }
    }

    public void writeCharacteristic(BluetoothGattCharacteristic characteristic,
                                    String data) {
        if (bluetoothAdapter == null || bluetoothGatt == null) {
            Log.w("bambi", "BluetoothAdapter not initialized");
            return;
        }

        Log.i("bambi", "characteristic " + characteristic.toString());
        try {
            Log.i("bambi", "data " + URLEncoder.encode(data, "utf-8"));

            characteristic.setValue(URLEncoder.encode(data, "utf-8"));

            bluetoothGatt.writeCharacteristic(characteristic);
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
    }
}
