package com.example.atheros.mywatch;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        SharedPreferences sharedPreferences = getSharedPreferences(getApplication().getPackageName(),Context.MODE_PRIVATE);
        String device_id = sharedPreferences.getString("device_id", null);

        Intent intent = null;

        if (device_id == null){
            intent = new Intent(this, LoginActivity.class);
        } else {
            intent = new Intent(this, HomeActivity.class);
            intent.putExtra("device_id", device_id);
        }
        startActivity(intent);
        finish();
    }
}
