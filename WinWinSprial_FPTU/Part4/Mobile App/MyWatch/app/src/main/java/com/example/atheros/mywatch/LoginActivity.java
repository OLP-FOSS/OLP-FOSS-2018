package com.example.atheros.mywatch;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.android.volley.AuthFailureError;
import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;

import java.util.HashMap;
import java.util.Map;

public class LoginActivity extends AppCompatActivity {

    EditText edtUser, edtPass;
    Button btnLogin;
    public String loginURL = null;
    RequestQueue requestQueue;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        mapping();
        loginURL = "http://"+ getApplicationContext().getString(R.string.ip) +":1880/login";
        requestQueue =  Volley.newRequestQueue(this);

        btnLogin.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String user = edtUser.getText().toString();
                if (user.isEmpty()){
                    Toast.makeText(LoginActivity.this, "Can not left user empty", Toast.LENGTH_SHORT).show();
                    return;
                }
                String pass = edtPass.getText().toString();
                if (pass.isEmpty()){
                    Toast.makeText(LoginActivity.this, "Can not left pass empty", Toast.LENGTH_SHORT).show();
                    return;
                }
                authenticate(user, pass);
            }
        });
    }

    void mapping(){
        edtUser = (EditText) findViewById(R.id.user);
        edtPass = (EditText) findViewById(R.id.pass);
        btnLogin = (Button) findViewById(R.id.login);
    }

    void authenticate(final String user, final String pass){
        StringRequest request = new StringRequest(Request.Method.POST, loginURL, new Response.Listener<String>() {
            @Override
            public void onResponse(String response) {
                SharedPreferences sharedPreferences = getSharedPreferences(getApplication().getPackageName(),Context.MODE_PRIVATE);
                SharedPreferences.Editor editor = sharedPreferences.edit();
                editor.putString("device_id", response);
                editor.commit();
                Toast.makeText(LoginActivity.this, response, Toast.LENGTH_SHORT).show();

                Intent intent = new Intent(LoginActivity.this, HomeActivity.class);
                intent.putExtra("device_id", response);
                startActivity(intent);
                finish();
            }
        }, new Response.ErrorListener() {
            @Override
            public void onErrorResponse(VolleyError error) {
                Toast.makeText(LoginActivity.this, "Error", Toast.LENGTH_SHORT).show();
            }
        }) {
            @Override
            public String getBodyContentType() {
                return "application/x-www-form-urlencoded; charset=UTF-8";
            }

            @Override
            protected Map<String, String> getParams() throws AuthFailureError {
                Map<String, String> postParam = new HashMap<String, String>();

                postParam.put("user", user);
                postParam.put("pass", pass);


                return postParam;
            }
        };
        requestQueue.add(request);

    }
}
