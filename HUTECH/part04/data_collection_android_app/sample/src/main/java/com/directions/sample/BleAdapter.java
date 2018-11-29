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
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;

import com.google.android.gms.common.api.GoogleApiClient;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
public class BleAdapter extends ArrayAdapter<BluetoothDevice> {

    private int resourceLayout;
    private Context mContext;
    private List<BluetoothDevice> datas = new ArrayList<>();

    public BleAdapter(Context context, int resource, List<BluetoothDevice> items) {
        super(context, resource, items);
        this.resourceLayout = resource;
        this.mContext = context;
        this.datas = items;
    }

    public void setData(List<BluetoothDevice> items) {
        this.datas = items;
        notifyDataSetChanged();
    }

    @Override
    public int getCount() {
        return datas.size();
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {

        View v = convertView;

        if (v == null) {
            LayoutInflater vi;
            vi = LayoutInflater.from(mContext);
            v = vi.inflate(resourceLayout, null);
        }

        BluetoothDevice p = datas.get(position);

        if (p != null) {
            TextView tvDeviceName = (TextView) v.findViewById(R.id.tv_device_name);
            TextView tvAddress = (TextView) v.findViewById(R.id.tv_address);

            tvDeviceName.setText("Name: " + p.getName());
            tvAddress.setText("Address: " + p.getAddress());
           }

        return v;
    }

}