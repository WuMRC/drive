package com.shemanigans.mime;

import android.app.Activity;
import android.app.Fragment;
import android.graphics.Color;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import com.androidplot.xy.BoundaryMode;
import com.androidplot.xy.LineAndPointFormatter;
import com.androidplot.xy.SimpleXYSeries;
import com.androidplot.xy.XYPlot;

public class BioimpFragment extends Fragment{

	/**
	 * The fragment argument representing the section number for this
	 * fragment.
	 */
	private static final String ARG_SECTION_NUMBER = "section_number";
	private static final int HISTORY_SIZE = 360;
	private XYPlot bioimpedancePlot = null;
	private SimpleXYSeries accelXseries = null;
	private SimpleXYSeries accelYseries = null;
	private SimpleXYSeries accelZseries = null;
	private SimpleXYSeries bioimpedanceSeries = null;
	private String mDeviceName;	
	private String mDeviceAddress;

	OnButtonClickedListener mCallback;

	/**
	 * Returns a new instance of this fragment for the given section
	 * number.
	 */
	public static BioimpFragment newInstance(int sectionNumber) {
		BioimpFragment fragment = new BioimpFragment();
		Bundle args = new Bundle();
		args.putInt(ARG_SECTION_NUMBER, sectionNumber);
		fragment.setArguments(args);
		return fragment;
	}

	public BioimpFragment() {
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		View rootView = null;

		switch (getArguments().getInt(ARG_SECTION_NUMBER)) {
		case 1:
			rootView = inflater.inflate(R.layout.fragment_long_term, container, false);
			TextView deviceName = (TextView) rootView.findViewById(R.id.device_name);
			TextView deviceAddress = (TextView) rootView.findViewById(R.id.device_address);
			Button exportToText = (Button) rootView.findViewById(R.id.export_to_text_frag);
			Button clearTextFile = (Button) rootView.findViewById(R.id.clear_text_file_frag);

			exportToText.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					exportToTextFrag();
				}
			});

			clearTextFile.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					clearTextFileFrag();
				}
			});

			deviceName.setText(mDeviceName);
			deviceAddress.setText(mDeviceAddress);

			bioimpedancePlot = (XYPlot) rootView.findViewById(R.id.bioimpedancePlot2);

			accelXseries = new SimpleXYSeries("AccelX");
			accelXseries.useImplicitXVals();
			accelYseries = new SimpleXYSeries("AccelY");
			accelYseries.useImplicitXVals();
			accelZseries = new SimpleXYSeries("AccelZ");
			accelZseries.useImplicitXVals();
			bioimpedanceSeries = new SimpleXYSeries("bioimpedance");
			bioimpedanceSeries.useImplicitXVals();

			bioimpedancePlot.setRangeBoundaries(0, 600, BoundaryMode.FIXED);
			bioimpedancePlot.setDomainBoundaries(0, 360, BoundaryMode.FIXED);

			bioimpedancePlot.addSeries(accelXseries, new LineAndPointFormatter(Color.CYAN, null, null, null));
			bioimpedancePlot.addSeries(accelYseries, new LineAndPointFormatter(Color.GREEN, null, null, null));
			bioimpedancePlot.addSeries(accelZseries, new LineAndPointFormatter(Color.MAGENTA, null, null, null));
			bioimpedancePlot.addSeries(bioimpedanceSeries, new LineAndPointFormatter(Color.YELLOW, null, null, null));
			bioimpedancePlot.setDomainStepValue(5);
			bioimpedancePlot.setTicksPerRangeLabel(3);
			bioimpedancePlot.setDomainLabel("Sample Index");
			bioimpedancePlot.getDomainLabelWidget().pack();
			bioimpedancePlot.setRangeLabel("Data");
			bioimpedancePlot.getRangeLabelWidget().pack(); 
			break;
		case 2:
			rootView = inflater.inflate(R.layout.fragment_placeholder, container, false);
			break;
		case 3:
			rootView = inflater.inflate(R.layout.fragment_placeholder, container, false);
			break;
		case 4:
			rootView = inflater.inflate(R.layout.fragment_placeholder, container, false);
			break;
		case 5:
			rootView = inflater.inflate(R.layout.fragment_placeholder, container, false);
			break;
		}
		return rootView;		
	}

	// Container Activity must implement this interface
	public interface OnButtonClickedListener {
		public void exportToText();
		public void clearTextFile();
	}

	@Override
	public void onAttach(Activity activity) {
		super.onAttach(activity);

		// This makes sure that the container activity has implemented
		// the callback interface. If not, it throws an exception
		try {
			mCallback = (OnButtonClickedListener) activity;
		} catch (ClassCastException e) {
			throw new ClassCastException(activity.toString()
					+ " must implement OnHeadlineSelectedListener");
		}

		((LongTerm) activity).onSectionAttached(
				getArguments().getInt(ARG_SECTION_NUMBER));
	}

	public void exportToTextFrag() {
		mCallback.exportToText();
	}

	public void clearTextFileFrag() {
		mCallback.clearTextFile();
	}

	public void updatePlot(double[] imp) {
		if (accelXseries.size() > HISTORY_SIZE) {		        	
			accelXseries.removeFirst();
			accelYseries.removeFirst();
			accelZseries.removeFirst();
			bioimpedanceSeries.removeFirst();
		}

		// add the latest history sample:
		accelXseries.addLast(null, imp[0]);
		accelYseries.addLast(null, imp[1]);
		accelZseries.addLast(null, imp[2]);
		bioimpedanceSeries.addLast(null, imp[3]);

		// redraw the Plots:
		bioimpedancePlot.redraw();	
	}

	public String getDeviceName() {
		return mDeviceName;
	}

	public void setDeviceName(String mDeviceName) {
		this.mDeviceName = mDeviceName;
	}

	public String getDeviceAddress() {
		return mDeviceAddress;
	}

	public void setDeviceAddress(String mDeviceAddress) {
		this.mDeviceAddress = mDeviceAddress;
	}

}
