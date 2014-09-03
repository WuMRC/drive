package com.shemanigans.mime;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.nio.channels.FileChannel;
import java.util.Calendar;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Binder;
import android.os.Environment;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;

public class ServiceBinder extends Service {

	private final static String TAG = BluetoothLeService.class.getSimpleName();
	private String mDeviceAddress;
	public String mDeviceName;
	public double[] values = {1, 2, 3, 4};
	private String data;
	private String[] imp = new String[500];
	private int i = 0;
	private int j = 0;
	Calendar c = Calendar.getInstance();
	private File DataDir = new File(Environment.getExternalStorageDirectory() + "/Mime/");
	private File hourly = new File(DataDir, "hourly.txt");
	private File daily  = new File(DataDir, "daily.txt");
	//private File complete  = new File(DataDir, "complete.txt");


	//private Calendar c = Calendar.getInstance();
	//File file = new File(this.getFilesDir(), "cache");

	private BluetoothLeService mBluetoothLeService;

	private final ServiceConnection mServiceConnection = new ServiceConnection() {

		@Override
		public void onServiceConnected(ComponentName componentName, IBinder service) {
			mBluetoothLeService = ((BluetoothLeService.LocalBinder) service).getService();
			mBluetoothLeService.clientConnected();
			Log.i(TAG, "Bound to BLE service. No of clients = " + mBluetoothLeService.getNumberOfBoundClients());
			//mBluetoothLeService.connect(mDeviceAddress);
			Log.i(TAG, mDeviceAddress);
		}

		@Override
		public void onServiceDisconnected(ComponentName componentName) {
			mBluetoothLeService.clientDisconnected();
			mBluetoothLeService = null;
		}
	};

	private final BroadcastReceiver mGattUpdateReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			final String action = intent.getAction();
			if (BluetoothLeService.ACTION_DATA_AVAILABLE_BIOIMPEDANCE.equals(action)) {
				values = intent.getDoubleArrayExtra(BluetoothLeService.EXTRA_DATA_BIOIMPEDANCE_DOUBLE);
				data = intent.getStringExtra(BluetoothLeService.EXTRA_DATA_BIOIMPEDANCE_STRING);
				
				// Store data in preallocated memory to conserve resources.
				if(i <= 499) {
					imp[i] = data;
				}
				else {
					i = -1;
					long start = System.nanoTime();
					Log.i(TAG, "Arrays full.");
					exportToText(imp, hourly);
					long end = System.nanoTime();
					Log.i(TAG, String.valueOf(end - start));
					if(fileCheck(daily)) {
						exportToText(imp, daily);
					}
					j++;
				}
				if(j >= 5) {
					j = 0;
					new Thread(new Runnable() {
						public void run() {
							try {

								if(fileCheck(daily)) {
									//check if daily exists. if so then append file to file.
									// {
									//}
								}
								// else, copy
								else {
									copyFile(hourly, daily);
									// TODO Delete old hourly data
									// Re-export to text initial
								}
							} 
							catch (IOException e) {
								e.printStackTrace();
							}
						}
					}).start();
				}
				i++;
			}
		}
	};


	@Override
	public void onCreate() {
		Intent gattServiceIntent = new Intent(this, BluetoothLeService.class);
		bindService(gattServiceIntent, mServiceConnection, BIND_AUTO_CREATE);
		registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter());
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		mDeviceAddress = intent.getStringExtra(DeviceControlActivity.EXTRA_DEVICE_ADDRESS_BINDER);
		mDeviceName = intent.getStringExtra(DeviceControlActivity.EXTRA_DEVICE_NAME_BINDER);
		DataDir.mkdir();
		fileCheckInitial();
		return START_STICKY;
	}

	public class LocalBinder extends Binder {
		ServiceBinder getService() {
			return ServiceBinder.this;
		}
	}

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}

	private final IBinder mBinder = new LocalBinder();

	@Override
	public void onDestroy() {
		unregisterReceiver(mGattUpdateReceiver);
		unbindService(mServiceConnection);
		mBluetoothLeService.clientDisconnected();
	}

	private static IntentFilter makeGattUpdateIntentFilter() {
		final IntentFilter intentFilter = new IntentFilter();
		intentFilter.addAction(BluetoothLeService.ACTION_GATT_CONNECTED);
		intentFilter.addAction(BluetoothLeService.ACTION_GATT_DISCONNECTED);
		intentFilter.addAction(BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED);
		intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE);
		intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE_BIOIMPEDANCE);
		return intentFilter;
	}


	public void exportToText(String[] value, File file) {
		// write on SD card file data in the text box

		final String[] writer = value;
		final File exported = file;

		new Thread(new Runnable() {
			public void run() {
				try {
					FileOutputStream fOut = new FileOutputStream(exported, true);
					OutputStreamWriter myOutWriter = new OutputStreamWriter(fOut);

					for (int i = 0; i < writer.length; i++) {
						myOutWriter.append(writer[i]);
						myOutWriter.append("\n");
					}

					myOutWriter.close();
					fOut.close();
				} 
				catch (Exception e) {
					Log.i(TAG, e.getMessage());
				}
			}
		}).start();

	}

	public void exportToTextInitial() {
		// write files to SD
		try {
			FileOutputStream fOutHourly = new FileOutputStream(hourly, true);
			OutputStreamWriter myOutWriter = new OutputStreamWriter(fOutHourly);
			hourly = new File(DataDir, "hourly.txt");
			myOutWriter.append(tableTitle());
			myOutWriter.close();
			fOutHourly.close();
			Toast.makeText(getBaseContext(),
					"Hourly file created",
					Toast.LENGTH_SHORT).show();
		} 
		catch (Exception e) {
			Toast.makeText(getBaseContext(), e.getMessage(),
					Toast.LENGTH_SHORT).show();
		}
	}

	public void fileCheckInitial() {
		if(hourly.exists() == false) {
			exportToTextInitial();
		}
	}

	public boolean fileCheck(File check) {
		if(check.exists() == false) {
			return false;
		}
		else {
			return true;
		}
	}

	public void copyFile(File source, File dest)
			throws IOException {
		FileChannel inputChannel = null;
		FileChannel outputChannel = null;
		try {
			inputChannel = new FileInputStream(source).getChannel();
			outputChannel = new FileOutputStream(dest).getChannel();
			outputChannel.transferFrom(inputChannel, 0, inputChannel.size());
		} 

		finally {
			inputChannel.close();
			outputChannel.close();
		}
	}

	/* Checks if external storage is available for read and write */
	public boolean isExternalStorageWritable() {
		String state = Environment.getExternalStorageState();
		if (Environment.MEDIA_MOUNTED.equals(state)) {
			return true;
		}
		return false;
	}

	/* Checks if external storage is available to at least read */
	public boolean isExternalStorageReadable() {
		String state = Environment.getExternalStorageState();
		if (Environment.MEDIA_MOUNTED.equals(state) ||
				Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) {
			return true;
		}
		return false;
	}

	public String tableTitle() {
		return 	fixedLengthString("X", 6) 
				+ fixedLengthString("Y", 6)
				+ fixedLengthString("Z", 6)
				+ fixedLengthString("Imp.", 7)
				+ "\n";
	}

	public static String fixedLengthString(String string, int length) {
		return String.format("%-"+length+ "s", string);
	}

}
