package com.directions.sample;

import android.Manifest;
import android.app.Activity;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.nio.channels.FileChannel;

import static butterknife.ButterKnife.Finder.arrayOf;

/**
 * Created by macbook on 11/17/18.
 */

public class MyDB{

    private MyDatabaseHelper dbHelper;

    private SQLiteDatabase database;

    private Context context;

    public final static String EMP_TABLE="smartglass"; // name of table

    public final static String EMP_ID="_id"; // id value for employee
    public final static String EMP_NAME="name";  // name of employee

    /**
     *
     * @param context
     */
    public MyDB(Context context){
        dbHelper = new MyDatabaseHelper(context);
        database = dbHelper.getWritableDatabase();
        this.context = context;
    }


    public long createRecords(String id, float accelX, float accelY, float accelZ, boolean isCorrect){
        ContentValues values = new ContentValues();
//        values.put(EMP_ID, id);
//        values.put(EMP_NAME, "zzz");
        values.put("valuex", accelX);
        values.put("valuey", accelY);
        values.put("valuez", accelZ);
        values.put("correct", isCorrect ? 1 : 0);
        return database.insert(EMP_TABLE, null, values);
    }

    public Cursor selectRecords() {
        String[] cols = new String[] {EMP_ID, "valuex", "valuey", "valuez", "correct"};
        Cursor mCursor = database.query(true, EMP_TABLE,cols,null
                , null, null, null, null, null);
        if (mCursor != null) {
            mCursor.moveToFirst();
        }
        return mCursor; // iterate to get each value.
    }

    public String getTableAsString() {
        Log.d("bambi", "getTableAsString called");
        String tableString = String.format("Table %s:\n", EMP_TABLE);
        Cursor allRows  = selectRecords();
        if (allRows.moveToFirst() ){
            String[] columnNames = allRows.getColumnNames();
            do {
                for (String name: columnNames) {
                    tableString += String.format("%s: %s\n", name,
                            allRows.getString(allRows.getColumnIndex(name)));
                }
                tableString += "\n";

            } while (allRows.moveToNext());
        }

        return tableString;
    }

    public boolean exportDatabse() {
        try {
            File sd = Environment.getExternalStorageDirectory();
            File data = Environment.getDataDirectory();

            if (sd.canWrite()) {
                String currentDBPath = "//data//"+context.getPackageName()+"//databases//"+MyDatabaseHelper.DATABASE_NAME+"";
                String backupDBPath = "smartglass.db";
                File currentDB = new File(data, currentDBPath);
                File backupDB = new File(sd, backupDBPath);

                if (currentDB.exists()) {
                    FileChannel src = new FileInputStream(currentDB).getChannel();
                    FileChannel dst = new FileOutputStream(backupDB).getChannel();
                    dst.transferFrom(src, 0, src.size());
                    src.close();
                    dst.close();
                }

                return true;
            } else {
                ActivityCompat.requestPermissions((Activity) context,
                        arrayOf(Manifest.permission.WRITE_EXTERNAL_STORAGE),
                        1996);
            }
        } catch (Exception e) {

        }

        return false;
    }
}