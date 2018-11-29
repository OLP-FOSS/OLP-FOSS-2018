package com.directions.sample;

import android.app.ProgressDialog;
import android.content.Intent;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.location.Address;
import android.location.Geocoder;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.support.v7.app.AppCompatActivity;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AutoCompleteTextView;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.directions.route.AbstractRouting;
import com.directions.route.Route;
import com.directions.route.RouteException;
import com.directions.route.Routing;
import com.directions.route.RoutingListener;
import com.directions.route.Segment;
import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.PendingResult;
import com.google.android.gms.common.api.ResultCallback;
import com.google.android.gms.location.LocationServices;
import com.google.android.gms.location.places.Place;
import com.google.android.gms.location.places.PlaceBuffer;
import com.google.android.gms.location.places.Places;
import com.google.android.gms.maps.CameraUpdate;
import com.google.android.gms.maps.CameraUpdateFactory;
import com.google.android.gms.maps.GoogleMap;
import com.google.android.gms.maps.MapsInitializer;
import com.google.android.gms.maps.OnMapReadyCallback;
import com.google.android.gms.maps.SupportMapFragment;
import com.google.android.gms.maps.model.BitmapDescriptorFactory;
import com.google.android.gms.maps.model.CameraPosition;
import com.google.android.gms.maps.model.LatLng;
import com.google.android.gms.maps.model.LatLngBounds;
import com.google.android.gms.maps.model.MarkerOptions;
import com.google.android.gms.maps.model.Polyline;
import com.google.android.gms.maps.model.PolylineOptions;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;

import butterknife.ButterKnife;
import butterknife.InjectView;
import butterknife.OnClick;

public class MainActivity extends AppCompatActivity implements SensorEventListener, RoutingListener, GoogleApiClient.OnConnectionFailedListener, GoogleApiClient.ConnectionCallbacks {
    protected GoogleMap map;
    protected LatLng start;
    protected LatLng end;
    @InjectView(R.id.start)
    AutoCompleteTextView starting;
    @InjectView(R.id.destination)
    AutoCompleteTextView destination;
    @InjectView(R.id.send)
    ImageView send;
    @InjectView(R.id.tv_current_segment)
    TextView tvCurrentSegment;
    private Location lastLocation;
    private static final String LOG_TAG = "MyActivity";
    protected GoogleApiClient mGoogleApiClient;
    private PlaceAutoCompleteAdapter mAdapter;
    private ProgressDialog progressDialog;
    private List<Polyline> polylines;
    private List<Segment> segmentList = new ArrayList<>();
    private Segment currentSegment;
    private static final int[] COLORS = new int[]{R.color.primary_dark, R.color.red, R.color.blue, R.color.accent, R.color.primary_dark_material_light};


    private void changeStatusBarColor(String color) {
        if (Build.VERSION.SDK_INT >= 21) {
            Window window = getWindow();
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            window.setStatusBarColor(Color.parseColor(color));
        }
    }

    private static final LatLngBounds BOUNDS_JAMAICA = new LatLngBounds(new LatLng(-57.965341647205726, 144.9987719580531),
            new LatLng(72.77492067739843, -9.998857788741589));

    /**
     * This activity loads a map and then displays the route and pushpins on it.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ButterKnife.inject(this);
//        getSupportActionBar().setDisplayShowHomeEnabled(true);
        getSupportActionBar().hide();

        changeStatusBarColor("#002297");


        sensorManager=(SensorManager) getSystemService(SENSOR_SERVICE);
        senseAccel = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);


        polylines = new ArrayList<>();
        mGoogleApiClient = new GoogleApiClient.Builder(this)
                .addApi(Places.GEO_DATA_API)
                .addApi(LocationServices.API)
                .addConnectionCallbacks(this)
                .addOnConnectionFailedListener(this)
                .build();
        MapsInitializer.initialize(this);
        mGoogleApiClient.connect();

        SupportMapFragment mapFragment = (SupportMapFragment) getSupportFragmentManager().findFragmentById(R.id.map);

        if (mapFragment == null) {
            mapFragment = SupportMapFragment.newInstance();
            getSupportFragmentManager().beginTransaction().replace(R.id.map, mapFragment).commit();
        }
        mapFragment.getMapAsync(new OnMapReadyCallback() {
            @Override
            public void onMapReady(GoogleMap googleMap) {
                map = googleMap;


                Location currentLocation = LocationServices.FusedLocationApi.getLastLocation(mGoogleApiClient);
                if (currentLocation != null) {
                    lastLocation = currentLocation;
                    Place place = getPlaceByLatlng(currentLocation.getLatitude(), currentLocation.getLongitude());
                    if (place == null) {
                        Toast.makeText(MainActivity.this, "current location is null", Toast.LENGTH_SHORT).show();
                    } else {
                        Log.d("bambi", place.getAddress().toString());
                        start = place.getLatLng();
                        starting.setText(place.getAddress());
                    }
                }


                mAdapter = new PlaceAutoCompleteAdapter(MainActivity.this, android.R.layout.simple_list_item_1,
                        mGoogleApiClient, BOUNDS_JAMAICA, null);


        /*
        * Updates the bounds being used by the auto complete adapter based on the position of the
        * map.
        * */
                map.setOnCameraChangeListener(new GoogleMap.OnCameraChangeListener() {
                    @Override
                    public void onCameraChange(CameraPosition position) {
                        LatLngBounds bounds = map.getProjection().getVisibleRegion().latLngBounds;
                        mAdapter.setBounds(bounds);
                    }
                });


                CameraUpdate center = CameraUpdateFactory.newLatLng(new LatLng(18.013610, -77.498803));
                CameraUpdate zoom = CameraUpdateFactory.zoomTo(16);

                map.moveCamera(center);
                map.animateCamera(zoom);

                LocationManager locationManager = (LocationManager) getSystemService(LOCATION_SERVICE);

                locationManager.requestLocationUpdates(
                        LocationManager.NETWORK_PROVIDER, 5000, 0,
                        new LocationListener() {
                            @Override
                            public void onLocationChanged(Location location) {

                                CameraUpdate center = CameraUpdateFactory.newLatLng(new LatLng(location.getLatitude(), location.getLongitude()));
                                CameraUpdate zoom = CameraUpdateFactory.zoomTo(16);

                                map.moveCamera(center);
                                map.animateCamera(zoom);


                                lastLocation = location;
                                checkCurrentSegmentAndDisplay(location);
                            }

                            @Override
                            public void onStatusChanged(String provider, int status, Bundle extras) {

                            }

                            @Override
                            public void onProviderEnabled(String provider) {

                            }

                            @Override
                            public void onProviderDisabled(String provider) {

                            }
                        });


                locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER,
                        3000, 0, new LocationListener() {
                            @Override
                            public void onLocationChanged(Location location) {
                                CameraUpdate center = CameraUpdateFactory.newLatLng(new LatLng(location.getLatitude(), location.getLongitude()));
                                CameraUpdate zoom = CameraUpdateFactory.zoomTo(16);

                                map.moveCamera(center);
                                map.animateCamera(zoom);


                                lastLocation = location;
                                checkCurrentSegmentAndDisplay(location);
                            }

                            @Override
                            public void onStatusChanged(String provider, int status, Bundle extras) {

                            }

                            @Override
                            public void onProviderEnabled(String provider) {

                            }

                            @Override
                            public void onProviderDisabled(String provider) {

                            }
                        });



        /*
        * Adds auto complete adapter to both auto complete
        * text views.
        * */
                starting.setAdapter(mAdapter);
                destination.setAdapter(mAdapter);


        /*
        * Sets the start and destination points based on the values selected
        * from the autocomplete text views.
        * */

                starting.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                    @Override
                    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {

                        final PlaceAutoCompleteAdapter.PlaceAutocomplete item = mAdapter.getItem(position);
                        final String placeId = String.valueOf(item.placeId);
                        Log.i(LOG_TAG, "Autocomplete item selected: " + item.description);

            /*
             Issue a request to the Places Geo Data API to retrieve a Place object with additional
              details about the place.
              */
                        PendingResult<PlaceBuffer> placeResult = Places.GeoDataApi
                                .getPlaceById(mGoogleApiClient, placeId);
                        placeResult.setResultCallback(new ResultCallback<PlaceBuffer>() {
                            @Override
                            public void onResult(PlaceBuffer places) {
                                if (!places.getStatus().isSuccess()) {
                                    // Request did not complete successfully
                                    Log.e(LOG_TAG, "Place query did not complete. Error: " + places.getStatus().toString());
                                    places.release();
                                    return;
                                }
                                // Get the Place object from the buffer.
                                final Place place = places.get(0);

                                start = place.getLatLng();
                            }
                        });

                    }
                });
                destination.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                    @Override
                    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {

                        final PlaceAutoCompleteAdapter.PlaceAutocomplete item = mAdapter.getItem(position);
                        final String placeId = String.valueOf(item.placeId);
                        Log.i(LOG_TAG, "Autocomplete item selected: " + item.description);

            /*
             Issue a request to the Places Geo Data API to retrieve a Place object with additional
              details about the place.
              */
                        PendingResult<PlaceBuffer> placeResult = Places.GeoDataApi
                                .getPlaceById(mGoogleApiClient, placeId);
                        placeResult.setResultCallback(new ResultCallback<PlaceBuffer>() {
                            @Override
                            public void onResult(PlaceBuffer places) {
                                if (!places.getStatus().isSuccess()) {
                                    // Request did not complete successfully
                                    Log.e(LOG_TAG, "Place query did not complete. Error: " + places.getStatus().toString());
                                    places.release();
                                    return;
                                }
                                // Get the Place object from the buffer.
                                final Place place = places.get(0);

                                end = place.getLatLng();
                            }
                        });

                    }
                });

        /*
        These text watchers set the start and end points to null because once there's
        * a change after a value has been selected from the dropdown
        * then the value has to reselected from dropdown to get
        * the correct location.
        * */
                starting.addTextChangedListener(new TextWatcher() {
                    @Override
                    public void beforeTextChanged(CharSequence s, int start, int count, int after) {

                    }

                    @Override
                    public void onTextChanged(CharSequence s, int startNum, int before, int count) {
//                        if (start != null) {
//                            start = null;
//                        }
                    }

                    @Override
                    public void afterTextChanged(Editable s) {

                    }
                });

                destination.addTextChangedListener(new TextWatcher() {
                    @Override
                    public void beforeTextChanged(CharSequence s, int start, int count, int after) {

                    }

                    @Override
                    public void onTextChanged(CharSequence s, int start, int before, int count) {


//                        if (end != null) {
//                            end = null;
//                        }
                    }

                    @Override
                    public void afterTextChanged(Editable s) {

                    }
                });
            }
        });
    }

    private Place getPlaceByLatlng(final double latitude, final double longitude) {
        Geocoder geocoder;
        final List<Address> addresses;
        geocoder = new Geocoder(this, Locale.getDefault());

        Place place = null;
        try {
            addresses = geocoder.getFromLocation(latitude, longitude, 1); // Here 1 represent max location result to returned, by documents it recommended 1 to 5

            place = new Place() {
                @Override
                public String getId() {
                    return null;
                }

                @Override
                public List<Integer> getPlaceTypes() {
                    return null;
                }

                @Override
                public CharSequence getAddress() {
                    String address = addresses.get(0).getAddressLine(0); // If any additional address line present than only, check with max available address lines by getMaxAddressLineIndex()
                    return address;
                }

                @Override
                public Locale getLocale() {
                    return addresses.get(0).getLocale();
                }

                @Override
                public CharSequence getName() {

                    return addresses.get(0).getFeatureName();
                }

                @Override
                public LatLng getLatLng() {
                    return new LatLng(latitude, longitude);
                }

                @Override
                public LatLngBounds getViewport() {
                    return null;
                }

                @Override
                public Uri getWebsiteUri() {
                    return null;
                }

                @Override
                public CharSequence getPhoneNumber() {
                    return null;
                }

                @Override
                public float getRating() {
                    return 0;
                }

                @Override
                public int getPriceLevel() {
                    return 0;
                }

                @Override
                public CharSequence getAttributions() {
                    return null;
                }

                @Override
                public Place freeze() {
                    return null;
                }

                @Override
                public boolean isDataValid() {
                    return true;
                }
            };
        } catch (IOException e) {
            e.printStackTrace();
        }

        return place;
    }

    @OnClick(R.id.send)
    public void sendRequest() {
        if (Util.Operations.isOnline(this)) {
            route();
        } else {
            Toast.makeText(this, "No internet connectivity", Toast.LENGTH_SHORT).show();
        }
    }


    public void checkCurrentSegmentAndDisplay(Location location) {
        if (segmentList.size() == 0 || location == null || location.getAccuracy() > 100) {
            return;
        }

        ArrayList<Segment> listOkeSegments = new ArrayList<>();
        ArrayList<Float> listOkeSegmentsDistance = new ArrayList<>();
        for (Segment segment : segmentList) {
            Location target = new Location("target");
            target.setLatitude(segment.startPoint().latitude);
            target.setLongitude(segment.startPoint().longitude);

            float distance = location.distanceTo(target);
            if (distance < 100) {
                listOkeSegments.add(segment);
                listOkeSegmentsDistance.add(distance);
            }
        }

        try {

            Float minDistance = 1000000000f;
            if(listOkeSegmentsDistance.size() == 0) {
                currentSegment= segmentList.get(0);
            }else {

                for (Float distance : listOkeSegmentsDistance) {
                    if (minDistance > distance) {
                        minDistance = distance;
                    }
                }
                currentSegment = listOkeSegments.get(listOkeSegmentsDistance.indexOf(minDistance));
            }



            tvCurrentSegment.setText(currentSegment.getInstruction());
            Log.d("bambi", currentSegment.getInstruction());
            String distanceStr = "";
            float distanceKm = currentSegment.getLength() / 1000;
            int distanceM = currentSegment.getLength() % 1000;
            if (distanceKm > 0) {
                distanceStr = String.format("%.02f", distanceKm) + "km";
            } else {
                distanceStr = distanceM + "m";
            }

            if (currentSegment.getInstruction().toLowerCase().startsWith("head")) {
                BLEManager.getInstance().writeCharacterristic("head_" + distanceStr);
            } else if (currentSegment.getInstruction().toLowerCase().startsWith("turn left")
                    || currentSegment.getInstruction().toLowerCase().startsWith("slight left")) {
                BLEManager.getInstance().writeCharacterristic("left_" + distanceStr);
            } else if (currentSegment.getInstruction().toLowerCase().startsWith("turn right")
                    || currentSegment.getInstruction().toLowerCase().startsWith("slight right")) {
                BLEManager.getInstance().writeCharacterristic("right_" + distanceStr);
            }
        }catch (Exception e) {
            e.printStackTrace();
        }
    }


    // Mang cac ky tu goc co dau
    private static char[] SOURCE_CHARACTERS = {'À', 'Á', 'Â', 'Ã', 'È', 'É',
            'Ê', 'Ì', 'Í', 'Ò', 'Ó', 'Ô', 'Õ', 'Ù', 'Ú', 'Ý', 'à', 'á', 'â',
            'ã', 'è', 'é', 'ê', 'ì', 'í', 'ò', 'ó', 'ô', 'õ', 'ù', 'ú', 'ý',
            'Ă', 'ă', 'Đ', 'đ', 'Ĩ', 'ĩ', 'Ũ', 'ũ', 'Ơ', 'ơ', 'Ư', 'ư', 'Ạ',
            'ạ', 'Ả', 'ả', 'Ấ', 'ấ', 'Ầ', 'ầ', 'Ẩ', 'ẩ', 'Ẫ', 'ẫ', 'Ậ', 'ậ',
            'Ắ', 'ắ', 'Ằ', 'ằ', 'Ẳ', 'ẳ', 'Ẵ', 'ẵ', 'Ặ', 'ặ', 'Ẹ', 'ẹ', 'Ẻ',
            'ẻ', 'Ẽ', 'ẽ', 'Ế', 'ế', 'Ề', 'ề', 'Ể', 'ể', 'Ễ', 'ễ', 'Ệ', 'ệ',
            'Ỉ', 'ỉ', 'Ị', 'ị', 'Ọ', 'ọ', 'Ỏ', 'ỏ', 'Ố', 'ố', 'Ồ', 'ồ', 'Ổ',
            'ổ', 'Ỗ', 'ỗ', 'Ộ', 'ộ', 'Ớ', 'ớ', 'Ờ', 'ờ', 'Ở', 'ở', 'Ỡ', 'ỡ',
            'Ợ', 'ợ', 'Ụ', 'ụ', 'Ủ', 'ủ', 'Ứ', 'ứ', 'Ừ', 'ừ', 'Ử', 'ử', 'Ữ',
            'ữ', 'Ự', 'ự',};

    // Mang cac ky tu thay the khong dau
    private static char[] DESTINATION_CHARACTERS = {'A', 'A', 'A', 'A', 'E',
            'E', 'E', 'I', 'I', 'O', 'O', 'O', 'O', 'U', 'U', 'Y', 'a', 'a',
            'a', 'a', 'e', 'e', 'e', 'i', 'i', 'o', 'o', 'o', 'o', 'u', 'u',
            'y', 'A', 'a', 'D', 'd', 'I', 'i', 'U', 'u', 'O', 'o', 'U', 'u',
            'A', 'a', 'A', 'a', 'A', 'a', 'A', 'a', 'A', 'a', 'A', 'a', 'A',
            'a', 'A', 'a', 'A', 'a', 'A', 'a', 'A', 'a', 'A', 'a', 'E', 'e',
            'E', 'e', 'E', 'e', 'E', 'e', 'E', 'e', 'E', 'e', 'E', 'e', 'E',
            'e', 'I', 'i', 'I', 'i', 'O', 'o', 'O', 'o', 'O', 'o', 'O', 'o',
            'O', 'o', 'O', 'o', 'O', 'o', 'O', 'o', 'O', 'o', 'O', 'o', 'O',
            'o', 'O', 'o', 'U', 'u', 'U', 'u', 'U', 'u', 'U', 'u', 'U', 'u',
            'U', 'u', 'U', 'u',};

    /**
     * Bo dau 1 ky tu
     *
     * @param ch
     * @return
     */
    public char removeAccent(char ch) {
        int index = Arrays.binarySearch(SOURCE_CHARACTERS, ch);
        if (index >= 0) {
            ch = DESTINATION_CHARACTERS[index];
        }
        return ch;
    }

    /**
     * Bo dau 1 chuoi
     *
     * @param s
     * @return
     */
    public String removeAccent(String s) {
        StringBuilder sb = new StringBuilder(s);
        for (int i = 0; i < sb.length(); i++) {
            sb.setCharAt(i, removeAccent(sb.charAt(i)));
        }
        return sb.toString();

    }


    public void route() {
        if (start == null || end == null) {
            if (start == null) {
                if (starting.getText().length() > 0) {
//                    starting.setError("Choose location from dropdown.");
                } else {
                    Toast.makeText(this, "Please choose a starting point.", Toast.LENGTH_SHORT).show();
                }
            }
            if (end == null) {
                if (destination.getText().length() > 0) {
//                    destination.setError("Choose location from dropdown.");
                } else {
                    Toast.makeText(this, "Please choose a destination.", Toast.LENGTH_SHORT).show();
                }
            }
        } else {
            progressDialog = ProgressDialog.show(this, "Please wait.",
                    "Fetching route information.", true);
            Routing routing = new Routing.Builder()
                    .travelMode(AbstractRouting.TravelMode.DRIVING)
                    .withListener(this)
                    .alternativeRoutes(false)
                    .language("en")
                    .waypoints(start, end)
                    .key("AIzaSyAZXZecC5_UzAfxw4DVIYqlEmRm4NlFr9M")
                    .build();
            routing.execute();
        }
    }


    @Override
    public void onRoutingFailure(RouteException e) {
        // The Routing request failed
        progressDialog.dismiss();
        if (e != null) {
            Toast.makeText(this, "Error: " + e.getMessage(), Toast.LENGTH_LONG).show();
        } else {
            Toast.makeText(this, "Something went wrong, Try again", Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    public void onRoutingStart() {
        // The Routing Request starts
    }

    @Override
    public void onRoutingSuccess(ArrayList<Route> route, int shortestRouteIndex) {
        if (route == null || route.size() == 0) {
            return;
        }

        segmentList.clear();

        progressDialog.dismiss();
        CameraUpdate center = CameraUpdateFactory.newLatLng(start);
        CameraUpdate zoom = CameraUpdateFactory.zoomTo(10);

        map.moveCamera(center);


        if (polylines.size() > 0) {
            for (Polyline poly : polylines) {
                poly.remove();
            }
        }

        polylines = new ArrayList<>();
        Route nearestRoute = route.get(0);
        //add route(s) to the map.
        for (int i = 0; i < route.size(); i++) {
            if (nearestRoute.getDistanceValue() > route.get(i).getDistanceValue()) {
                nearestRoute = route.get(i);
            }
        }

        if (nearestRoute.getSegments().size() == 0) {
            return;
        }

        PolylineOptions polyOptions = new PolylineOptions();
        polyOptions.color(getResources().getColor(COLORS[1]));
        polyOptions.width(10);
        polyOptions.addAll(nearestRoute.getPoints());
        Polyline polyline = map.addPolyline(polyOptions);
        polylines.add(polyline);

        Toast.makeText(getApplicationContext(), "Distance - " + nearestRoute.getDistanceValue() + ": duration - " + nearestRoute.getDurationValue(), Toast.LENGTH_SHORT).show();


        // Start marker
        MarkerOptions options = new MarkerOptions();
        options.position(start);
        options.icon(BitmapDescriptorFactory.fromResource(R.drawable.start_blue));
        map.addMarker(options);

        // End marker
        options = new MarkerOptions();
        options.position(end);
        options.icon(BitmapDescriptorFactory.fromResource(R.drawable.end_green));
        map.addMarker(options);

        segmentList = nearestRoute.getSegments();
        currentSegment = segmentList.get(0);
        tvCurrentSegment.setText(currentSegment.getInstruction());


        checkCurrentSegmentAndDisplay(lastLocation);
    }


    @Override
    protected void onResume() {
        super.onResume();
        sensorManager.registerListener(this, senseAccel, SensorManager.SENSOR_DELAY_NORMAL);


        checkCurrentSegmentAndDisplay(lastLocation);
    }

    @Override
    protected void onPause() {
        super.onPause();
        sensorManager.unregisterListener(this);

    }

    @Override
    public void onRoutingCancelled() {
        Log.i(LOG_TAG, "Routing was cancelled.");
    }

    @Override
    public void onConnectionFailed(ConnectionResult connectionResult) {

        Log.v(LOG_TAG, connectionResult.toString());
    }

    @Override
    public void onConnected(Bundle bundle) {
        Location currentLocation = LocationServices.FusedLocationApi.getLastLocation(mGoogleApiClient);
        if (currentLocation != null) {

            lastLocation = currentLocation;
            Place place = getPlaceByLatlng(currentLocation.getLatitude(), currentLocation.getLongitude());
            if (place == null) {
                Toast.makeText(MainActivity.this, "current location is null", Toast.LENGTH_SHORT).show();
            } else {
                Log.d("bambi", place.getAddress().toString());
                start = place.getLatLng();
                starting.setText(place.getAddress());
            }
        }
    }

    @Override
    public void onConnectionSuspended(int i) {

    }

    int index = 0;
    private SensorManager sensorManager;
    private Sensor senseAccel;
    float[] accelValuesX = new float[128];
    float[] accelValuesY = new float[128];
    float[] accelValuesZ = new float[128];
    float[] detectedValues = new float[3];
    @Override
    public void onSensorChanged(SensorEvent sensorEvent) {
        // TODO Auto-generated method stub
        Sensor mySensor = sensorEvent.sensor;

        if (mySensor.getType() == Sensor.TYPE_ACCELEROMETER) {
            index++;
            accelValuesX[index] = sensorEvent.values[0];
            accelValuesY[index] = sensorEvent.values[1];
            accelValuesZ[index] = sensorEvent.values[2];


            if(index >= 127){
                index = 0;
            }


            accelValuesX[index] = sensorEvent.values[0];
            accelValuesY[index] = sensorEvent.values[1];
            accelValuesZ[index] = sensorEvent.values[2];

            double rootSquare=Math.sqrt(Math.pow(accelValuesX[index],2)+Math.pow(accelValuesY[index],2)+Math.pow(accelValuesZ[index],2));
            if(rootSquare<2.0)
            {

                index = 0;
                sensorManager.unregisterListener(this);
                Toast.makeText(this, "Fall detected", Toast.LENGTH_SHORT).show();
                detectedValues = sensorEvent.values;


                Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("smartglass://people/1"));
                startActivity(browserIntent);
            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {

    }
}
