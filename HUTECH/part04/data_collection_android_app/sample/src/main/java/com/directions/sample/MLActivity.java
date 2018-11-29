package com.directions.sample;

import android.app.ProgressDialog;
import android.content.Intent;
import android.database.Cursor;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.UUID;

public class MLActivity extends AppCompatActivity implements SensorEventListener {
    private SensorManager sensorManager;
    private Sensor senseAccel;
    float[] accelValuesX = new float[128];
    float[] accelValuesY = new float[128];
    float[] accelValuesZ = new float[128];
    float[] detectedValues = new float[3];
    private TextView tvX, tvY, tvZ;
    private LinearLayout lnResult;
    private Button btnCorrect, btnNotCorrect, btnViewDatabase, btnUploadDatabase;
    int index = 0;
    private MyDB myDB;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ml);

        myDB = new MyDB(this);
        btnViewDatabase = (Button) findViewById(R.id.btn_view_database);
        btnUploadDatabase = (Button) findViewById(R.id.btn_upload_database);
        tvX = (TextView) findViewById(R.id.tv_x);
        tvY = (TextView) findViewById(R.id.tv_y);
        tvZ = (TextView) findViewById(R.id.tv_z);
        lnResult = (LinearLayout) findViewById(R.id.ln_result);
        btnCorrect = (Button) findViewById(R.id.btn_correct);


        btnUploadDatabase.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                try {
                    exportCSV();

                    new Handler().postDelayed(new Runnable() {
                        @Override
                        public void run() {
                           new LongOperation().execute();
                        }
                    }, 1000);
                } catch (IOException e) {
                    e.printStackTrace();
                    Toast.makeText(MLActivity.this, "Export failed", Toast.LENGTH_SHORT).show();
                }
            }
        });
        
        btnViewDatabase.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                boolean isSuccess = myDB.exportDatabse();

                if(isSuccess) {
                    Toast.makeText(MLActivity.this, "Opening database viewer", Toast.LENGTH_SHORT).show();
                    new Handler().postDelayed(new Runnable() {
                        @Override
                        public void run() {
                            Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("viewdatabase://people/1"));
                            startActivity(browserIntent);
                        }
                    }, 1000);
                }
            }
        });

        btnCorrect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                btnCorrect.setEnabled(false);
                btnNotCorrect.setEnabled(false);
                sensorManager.registerListener(MLActivity.this, senseAccel, SensorManager.SENSOR_DELAY_NORMAL);

                myDB.createRecords(UUID.randomUUID().toString(), detectedValues[0], detectedValues[1], detectedValues[2], true);
            }
        });
        btnNotCorrect = (Button) findViewById(R.id.btn_not_correct);
        btnNotCorrect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                btnCorrect.setEnabled(false);
                btnNotCorrect.setEnabled(false);
                sensorManager.registerListener(MLActivity.this, senseAccel, SensorManager.SENSOR_DELAY_NORMAL);

                myDB.createRecords(UUID.randomUUID().toString(), detectedValues[0], detectedValues[1], detectedValues[2], false);
            }
        });
        sensorManager=(SensorManager) getSystemService(SENSOR_SERVICE);
        senseAccel = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        sensorManager.registerListener(this, senseAccel, SensorManager.SENSOR_DELAY_NORMAL);

    }

    public void exportCSV() throws IOException {
        {

            File folder = new File(Environment.getExternalStorageDirectory()
                    + "/Folder");

            boolean var = false;
            if (!folder.exists())
                var = folder.mkdir();

            System.out.println("" + var);


            final String filename = folder.toString() + "/" + "data.csv";


            // show waiting screen
            CharSequence contentTitle = getString(R.string.app_name);
            final ProgressDialog progDailog = ProgressDialog.show(
                    MLActivity.this, contentTitle, "Uploading...",
                    true);//please wait
            final Handler handler = new Handler() {
                @Override
                public void handleMessage(Message msg) {




                }
            };

            new Thread() {
                public void run() {
                    try {

                        FileWriter fw = new FileWriter(filename);

                        Cursor cursor =  myDB.selectRecords();
                        fw.append("x");
                        fw.append(',');

                        fw.append("y");
                        fw.append(',');

                        fw.append("z");
                        fw.append(',');

                        fw.append("correct");
                        fw.append(',');

                        fw.append('\n');

                        if (cursor.moveToFirst()) {
                            do {
                                fw.append(cursor.getString(cursor.getColumnIndex("valuex")));
                                fw.append(',');
                                fw.append(cursor.getString(cursor.getColumnIndex("valuey")));
                                fw.append(',');
                                fw.append(cursor.getString(cursor.getColumnIndex("valuez")));
                                fw.append(',');
                                fw.append(cursor.getString(cursor.getColumnIndex("correct")));
                                fw.append(',');
                                fw.append('\n');

                            } while (cursor.moveToNext());
                        }
                        if (cursor != null && !cursor.isClosed()) {
                            cursor.close();
                        }

                        // fw.flush();
                        fw.close();

                    } catch (Exception e) {
                    }
                    handler.sendEmptyMessage(0);
                    progDailog.dismiss();

                }
            }.start();

        }
    }

    @Override
    public void onSensorChanged(SensorEvent sensorEvent) {
        // TODO Auto-generated method stub
        Sensor mySensor = sensorEvent.sensor;

        if (mySensor.getType() == Sensor.TYPE_ACCELEROMETER) {
            index++;
            accelValuesX[index] = sensorEvent.values[0];
            accelValuesY[index] = sensorEvent.values[1];
            accelValuesZ[index] = sensorEvent.values[2];

            tvX.setText("X: " + sensorEvent.values[0]);
            tvY.setText("Y: " + sensorEvent.values[1]);
            tvZ.setText("Z: " + sensorEvent.values[2]);

            accelValuesX[index] = sensorEvent.values[0];
            accelValuesY[index] = sensorEvent.values[1];
            accelValuesZ[index] = sensorEvent.values[2];

            if(index >= 127){
                index = 0;

                double meanX, meanY, meanZ;
                double totalX = 0, totalY = 0, totalZ = 0;
                for(int i = 0 ; i < accelValuesX.length ; i++) {
                    totalX += accelValuesX[i];
                    totalY += accelValuesY[i];
                    totalZ += accelValuesZ[i];
                }
                meanX = totalX / accelValuesX.length;
                meanY = totalY / accelValuesY.length;
                meanZ = totalZ / accelValuesZ.length;

                double rootSquare=Math.sqrt(Math.pow(meanX,2)+Math.pow(meanY,2)+Math.pow(meanZ,2));
                if(rootSquare<2.0)
                {
                    index = 0;
                    sensorManager.unregisterListener(this);
                    Toast.makeText(this, "Fall detected", Toast.LENGTH_SHORT).show();
                    detectedValues = sensorEvent.values;


                    btnCorrect.setEnabled(true);
                    btnNotCorrect.setEnabled(true);
                }
            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {

    }


//    public void callFallRecognition(){
//        for(int i=11;i<128;i++){
//            double rootSquare=Math.sqrt(Math.pow(accelValuesX[index],2)+Math.pow(accelValuesY[index],2)+Math.pow(accelValuesZ[index],2));
//            if(rootSquare < 2.0)
//            {
//                clearData();
//                Toast.makeText(this, "Fall detected", Toast.LENGTH_SHORT).show();
//                break;
//            }
//
//        }
//    }

    public void callFallRecognition(){
        float prev = 0;
        float curr = 0;
        prev = 10;
        for(int i=11;i<128;i++){
            curr = accelValuesZ[i];
            if(Math.abs(prev - curr) > 18 ){
                Toast.makeText(this, "Fall detected", Toast.LENGTH_LONG).show();
                clearData();
            }

        }


    }

    private void clearData() {
        for (int i = 0 ; i < accelValuesX.length ; i++) {
            accelValuesX[i] = 1;
        }
        for (int i = 0 ; i < accelValuesY.length ; i++) {
            accelValuesY[i] = 1;
        }
        for (int i = 0 ; i < accelValuesZ.length ; i++) {
            accelValuesZ[i] = 1;
        }
    }




    public String multipartRequest(String urlTo, Map<String, String> parmas, String filepath, String filefield, String fileMimeType) {
        HttpURLConnection connection = null;
        DataOutputStream outputStream = null;
        InputStream inputStream = null;

        String twoHyphens = "--";
        String boundary = "*****" + Long.toString(System.currentTimeMillis()) + "*****";
        String lineEnd = "\r\n";

        String result = "";

        int bytesRead, bytesAvailable, bufferSize;
        byte[] buffer;
        int maxBufferSize = 1 * 1024 * 1024;

        String[] q = filepath.split("/");
        int idx = q.length - 1;

        try {
            File file = new File(filepath);
            FileInputStream fileInputStream = new FileInputStream(file);

            URL url = new URL(urlTo);
            connection = (HttpURLConnection) url.openConnection();

            connection.setDoInput(true);
            connection.setDoOutput(true);
            connection.setUseCaches(false);

            connection.setRequestMethod("POST");
            connection.setRequestProperty("Connection", "Keep-Alive");
            connection.setRequestProperty("User-Agent", "Android Multipart HTTP Client 1.0");
            connection.setRequestProperty("Content-Type", "multipart/form-data; boundary=" + boundary);

            outputStream = new DataOutputStream(connection.getOutputStream());
            outputStream.writeBytes(twoHyphens + boundary + lineEnd);
            outputStream.writeBytes("Content-Disposition: form-data; name=\"" + filefield + "\"; filename=\"" + q[idx] + "\"" + lineEnd);
            outputStream.writeBytes("Content-Type: " + fileMimeType + lineEnd);
            outputStream.writeBytes("Content-Transfer-Encoding: binary" + lineEnd);

            outputStream.writeBytes(lineEnd);

            bytesAvailable = fileInputStream.available();
            bufferSize = Math.min(bytesAvailable, maxBufferSize);
            buffer = new byte[bufferSize];

            bytesRead = fileInputStream.read(buffer, 0, bufferSize);
            while (bytesRead > 0) {
                outputStream.write(buffer, 0, bufferSize);
                bytesAvailable = fileInputStream.available();
                bufferSize = Math.min(bytesAvailable, maxBufferSize);
                bytesRead = fileInputStream.read(buffer, 0, bufferSize);
            }

            outputStream.writeBytes(lineEnd);

            // Upload POST Data
            Iterator<String> keys = parmas.keySet().iterator();
            while (keys.hasNext()) {
                String key = keys.next();
                String value = parmas.get(key);

                outputStream.writeBytes(twoHyphens + boundary + lineEnd);
                outputStream.writeBytes("Content-Disposition: form-data; name=\"" + key + "\"" + lineEnd);
                outputStream.writeBytes("Content-Type: text/plain" + lineEnd);
                outputStream.writeBytes(lineEnd);
                outputStream.writeBytes(value);
                outputStream.writeBytes(lineEnd);
            }

            outputStream.writeBytes(twoHyphens + boundary + twoHyphens + lineEnd);


            if (200 != connection.getResponseCode()) {
                Toast.makeText(this, "Failed to upload code:" + connection.getResponseCode() + " " + connection.getResponseMessage(), Toast.LENGTH_SHORT).show();
                return null;
            }

            inputStream = connection.getInputStream();

            result = this.convertStreamToString(inputStream);

            fileInputStream.close();
            inputStream.close();
            outputStream.flush();
            outputStream.close();

            return result;
        } catch (Exception e) {
            Toast.makeText(this, "Upload file error", Toast.LENGTH_SHORT).show();
            Log.e("bambi", "bambi loi ne" + e.toString());
            return null;
        }

    }

    private String convertStreamToString(InputStream is) {
        BufferedReader reader = new BufferedReader(new InputStreamReader(is));
        StringBuilder sb = new StringBuilder();

        String line = null;
        try {
            while ((line = reader.readLine()) != null) {
                sb.append(line);
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                is.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return sb.toString();
    }



    private class LongOperation extends AsyncTask<String, Void, String> {

        @Override
        protected String doInBackground(String... params) {
            //setup params
            Map<String, String> paramsa = new HashMap<String, String>(2);


            File folder = new File(Environment.getExternalStorageDirectory()
                    + "/Folder");

            boolean var = false;
            if (!folder.exists())
                var = folder.mkdir();

            System.out.println("" + var);


            final String filename = folder.toString() + "/" + "data.csv";


            String result = multipartRequest("https://pythonapidemo.herokuapp.com/upload", paramsa, filename, "file", "csv");
            Log.d("bambi ne", "bambi " + (result == null ? "null roi" : result));
            return "Executed";
        }

        @Override
        protected void onPostExecute(String result) {
            Toast.makeText(MLActivity.this, "Upload successfully", Toast.LENGTH_SHORT).show();
        }
    }


    @Override
    protected void onResume() {
        super.onResume();
        sensorManager.registerListener(this, senseAccel, SensorManager.SENSOR_DELAY_NORMAL);

    }

    @Override
    protected void onPause() {
        super.onPause();
        sensorManager.unregisterListener(this);

    }
}
