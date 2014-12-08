package com.shemanigans.mime;

import java.util.HashMap;

/**
 * This class includes a small subset of standard GATT attributes for demonstration purposes.
 */
public class SampleGattAttributes {

	private static HashMap<String, String> attributes = new HashMap<String, String>();

	// Services
	public static String CLIENT_CHARACTERISTIC_CONFIG = "00002902-0000-1000-8000-00805f9b34fb";
	public static String HEART_RATE_SERVICE = "0000180d-0000-1000-8000-00805f9b34fb";
	public static String DEVICE_INFORMATION_SERVICE = "0000180a-0000-1000-8000-00805f9b34fb";
	public static String GENERIC_ACCESS_SERVICE = "00001800-0000-1000-8000-00805f9b34fb";
	public static String FREQUENCY_SWEEP_SERVICE = "645948cb-9c8c-42b6-87a1-b3512a50362c";
	public static String BIOIMPEDANCE_TRANSCEIVER_SERVICE = "195ae58a-437a-489b-b0cd-b7c9c394bae4";

	// Characteristics
	public static String DEVICE_NAME = "00002a00-0000-1000-8000-00805f9b34fb";
	public static String DEVICE_APPEARANCE = "00002a01-0000-1000-8000-00805f9b34fb";
	public static String MANUFACTURER_NAME = "00002a29-0000-1000-8000-00805f9b34fb";
	public static String MODEL_NUMBER = "00002a24-0000-1000-8000-00805f9b34fb";
	public static String SERIAL_NUMBER = "00002a25-0000-1000-8000-00805f9b34fb";

	public static String HEART_RATE_MEASUREMENT = "00002a37-0000-1000-8000-00805f9b34fb";
	public static String BIOIMPEDANCE_DATA = "21819ab0-c937-4188-b0db-b9621e1696cd";
	public static String SAMPLE_RATE = "5fc569a0-74a9-4fa4-b8b7-8354c86e45a4";
	public static String AC_FREQ = "07e028ea-218f-4c10-8c83-9fbdccd25304";

	static {
		
		// Sample Services.
		attributes.put(HEART_RATE_SERVICE, "Heart Rate Service");
		attributes.put(DEVICE_INFORMATION_SERVICE, "Device Information Service");
		attributes.put(GENERIC_ACCESS_SERVICE, "Generic Access Service");
		attributes.put(FREQUENCY_SWEEP_SERVICE, "Frequency Sweep Service");
		attributes.put(BIOIMPEDANCE_TRANSCEIVER_SERVICE, "Bioimpedance Transciever Service");


		// Sample Characteristics.
		attributes.put(HEART_RATE_MEASUREMENT, "Heart Rate Measurement");
		attributes.put(DEVICE_NAME, "Device Name");
		attributes.put(DEVICE_APPEARANCE, "Appearance");
		attributes.put(MANUFACTURER_NAME, "Manufacturer Name String");
		attributes.put(MODEL_NUMBER, "Model Number String");
		attributes.put(SERIAL_NUMBER, "Serial Number String");
		attributes.put(SAMPLE_RATE, "Sample Rate");
		attributes.put(BIOIMPEDANCE_DATA, "Bioimpedance Data");
		attributes.put(AC_FREQ, "AC Frequency");

	}

	public static String lookup(String uuid, String defaultName) {
		String name = attributes.get(uuid);
		/*if (name == null) {
			return defaultName;
		}
		else {
			return name;
		}*/
		return name == null ? defaultName : name;
	}
}