package com.example.atheros.mywatch;


import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.android.volley.AuthFailureError;
import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import java.util.HashMap;
import java.util.Map;

public class Status extends Fragment {



    final String topicPub = "android";
    TextView txtTime;
    TextView txtStatus;
    RequestQueue requestQueue;
    public String statusURL = null;

    public Status() {
        // Required empty public constructor
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_status, container, false);
        mapping(view);
        statusURL = "http://"+ getContext().getApplicationContext().getString(R.string.ip) +":1880/status";
        requestQueue =  Volley.newRequestQueue(getContext());
        HomeActivity.client.setCallback(new MqttCallback() {
            @Override
            public void connectionLost(Throwable cause) {

            }

            @Override
            public void messageArrived(String topic, MqttMessage message) throws Exception {
                String[] result = message.toString().split("-");
                txtStatus.setText(result[0]);
                txtTime.setText(result[1]);
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {

            }
        });
        getData();

        return view;
    }

    void mapping(View view){

        txtTime = (TextView) view.findViewById(R.id.time);
        txtStatus = (TextView) view.findViewById(R.id.status);
    }


    public void updateTime(String newTime){
        txtTime.setText(newTime);
    }


    void getData(){
        StringRequest request = new StringRequest(Request.Method.POST, statusURL, new Response.Listener<String>() {
            @Override
            public void onResponse(String response) {
                String[] result = response.split("-");
                txtStatus.setText(result[0]);
                txtTime.setText(result[1]);
            }
        }, new Response.ErrorListener() {
            @Override
            public void onErrorResponse(VolleyError error) {
                Toast.makeText(getActivity(), "Error", Toast.LENGTH_SHORT).show();
            }
        }) {
            @Override
            public String getBodyContentType() {
                return "application/x-www-form-urlencoded; charset=UTF-8";
            }

            @Override
            protected Map<String, String> getParams() throws AuthFailureError {
                Map<String, String> postParam = new HashMap<String, String>();

                postParam.put("device_id", HomeActivity.device_id);

                return postParam;
            }
        };
        requestQueue.add(request);

    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        super.onCreateOptionsMenu(menu, inflater);
    }
}
