package com.shemanigans.mime;

import android.app.Activity;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import com.androidplot.xy.BoundaryMode;
import com.androidplot.xy.LineAndPointFormatter;
import com.androidplot.xy.SimpleXYSeries;
import com.androidplot.xy.XYPlot;

public class BioimpFragment extends Fragment{ // This fragment presents information for the class LongTerm

	/**
	 * The fragment argument representing the section number for this
	 * fragment.
	 */
	private static final String ARG_SECTION_NUMBER = "section_number";
	private static final int HISTORY_SIZE = 360;
	private SimpleXYSeries accelXseries = null;
	private SimpleXYSeries accelYseries = null;
	private SimpleXYSeries accelZseries = null;
	private SimpleXYSeries bioimpedanceSeries = null;

	private String mDeviceName;	
	private String mDeviceAddress;
	private int sampleRate = 50;
	private byte startFreq = 0;
	private byte stepSize = 0;
	private byte numOfIncrements = 0;

	private TextView deviceName;
	private TextView deviceAddress;
	private TextView sampleRateTextView;
	private TextView startFreqTextView;
	private TextView stepSizeTextView;
	private TextView numOfIncrementsTextView;
	private Button exportToText;
	private Button clearTextFile;
	private XYPlot bioimpedancePlot = null;


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
			initializeViewComponentsLiveData(rootView);
			deviceName.setText(mDeviceName);
			deviceAddress.setText(mDeviceAddress);
			setSampleRateTextView(sampleRate);
			setAcFreqTextViewParams(startFreq, stepSize, numOfIncrements);

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
			setupPlot();

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
			rootView = inflater.inflate(R.layout.fragment_back_to_scanning, container, false);
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
					+ " must implement OnButtonClickedListener");
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

	public void setDeviceName(String mDeviceName) {
		this.mDeviceName = mDeviceName;
	}

	public void setDeviceAddress(String mDeviceAddress) {
		this.mDeviceAddress = mDeviceAddress;
	}

	public void updateSampleRate(int sampleRate) {
		this.sampleRate = sampleRate;
	}

	public void UpdateFrequencyParams(byte startFreq, byte stepSize, byte numOfIncrements) {
		this.startFreq = startFreq;
		this.stepSize = stepSize;
		this.numOfIncrements = numOfIncrements;
	}

	public void setSampleRateTextView(int sampleRate) {
		sampleRateTextView.setText(String.valueOf(sampleRate) + " Hz");
	}

	public void setAcFreqTextViewParams(byte startFreq, byte stepSize, byte numOfIncrements) {
		startFreqTextView.setText(String.valueOf(startFreq) + " KHz");
		if(stepSize == 0) {
			stepSizeTextView.setText(R.string.not_applicable);
		}
		else {
			stepSizeTextView.setText(String.valueOf(stepSize) + " KHz");
		}
		if(numOfIncrements == 0) {
			numOfIncrementsTextView.setText(R.string.not_applicable);
		}
		else {
			numOfIncrementsTextView.setText(String.valueOf(numOfIncrements));
		}
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

	private void initializeViewComponentsLiveData(View rootView) {		
		deviceName = (TextView) rootView.findViewById(R.id.device_name);
		deviceAddress = (TextView) rootView.findViewById(R.id.device_address);
		exportToText = (Button) rootView.findViewById(R.id.export_to_text_frag);
		clearTextFile = (Button) rootView.findViewById(R.id.clear_text_file_frag);
		bioimpedancePlot = (XYPlot) rootView.findViewById(R.id.bioimpedancePlot);
		sampleRateTextView = (TextView) rootView.findViewById(R.id.sample_rate);
		startFreqTextView = (TextView) rootView.findViewById(R.id.start_freq);
		stepSizeTextView = (TextView) rootView.findViewById(R.id.step_size);
		numOfIncrementsTextView = (TextView) rootView.findViewById(R.id.num_of_increments);
	}

	private void setupPlot() {
		Paint bgPaint = new Paint();
		bgPaint.setColor(Color.parseColor("#d8d8d8"));
		bgPaint.setStyle(Paint.Style.FILL);

		accelXseries = new SimpleXYSeries("X");
		accelXseries.useImplicitXVals();
		accelYseries = new SimpleXYSeries("Y");
		accelYseries.useImplicitXVals();
		accelZseries = new SimpleXYSeries("Z");
		accelZseries.useImplicitXVals();
		bioimpedanceSeries = new SimpleXYSeries("Ω");
		bioimpedanceSeries.useImplicitXVals();

		bioimpedancePlot.setRangeBoundaries(-100, 600, BoundaryMode.FIXED);
		bioimpedancePlot.setDomainBoundaries(0, 360, BoundaryMode.FIXED);

		// Format general area
		bioimpedancePlot.setBackgroundColor(Color.WHITE);
		bioimpedancePlot.getBackgroundPaint().set(bgPaint);
		bioimpedancePlot.getBackgroundPaint().setColor(Color.parseColor("#d8d8d8"));
		bioimpedancePlot.getGraphWidget().getBackgroundPaint().setColor(Color.parseColor("#d8d8d8"));
		bioimpedancePlot.getGraphWidget().setGridBackgroundPaint(null);
		bioimpedancePlot.setBorderStyle(XYPlot.BorderStyle.SQUARE, null, null);
		bioimpedancePlot.getGraphWidget().setPadding(12, 12, 12, 12);
		bioimpedancePlot.getTitleWidget().setText("");

		bioimpedancePlot.setBorderPaint(bgPaint);
		//bioimpedancePlot.getGraphWidget().getBorderPaint().setColor(Color.TRANSPARENT);

		// Format domain
		bioimpedancePlot.getDomainLabelWidget().getLabelPaint().setColor(Color.parseColor("#006bb2"));
		bioimpedancePlot.getDomainLabelWidget().getLabelPaint().setTextSize(20);
		bioimpedancePlot.getGraphWidget().getDomainLabelPaint().setColor(Color.BLACK);
		bioimpedancePlot.getGraphWidget().getDomainLabelPaint().setTextSize(20);
		bioimpedancePlot.getGraphWidget().getDomainOriginLabelPaint().setColor(Color.BLACK);
		bioimpedancePlot.getGraphWidget().getDomainOriginLinePaint().setColor(Color.BLACK);
		bioimpedancePlot.getGraphWidget().getDomainGridLinePaint().setColor(Color.TRANSPARENT);
		bioimpedancePlot.setDomainStepValue(5);
		bioimpedancePlot.setDomainLabel("Sample Index");
		bioimpedancePlot.getDomainLabelWidget().pack();


		// Format range
		bioimpedancePlot.getRangeLabelWidget().getLabelPaint().setColor(Color.parseColor("#006bb2"));
		bioimpedancePlot.getRangeLabelWidget().getLabelPaint().setTextSize(20);
		bioimpedancePlot.getGraphWidget().getRangeLabelPaint().setColor(Color.BLACK);
		bioimpedancePlot.getGraphWidget().getRangeLabelPaint().setTextSize(20);
		bioimpedancePlot.getGraphWidget().getRangeOriginLabelPaint().setColor(Color.BLACK);
		bioimpedancePlot.getGraphWidget().getRangeOriginLinePaint().setColor(Color.BLACK);
		bioimpedancePlot.getGraphWidget().getRangeGridLinePaint().setColor(Color.TRANSPARENT);
		bioimpedancePlot.getGraphWidget().getRangeSubGridLinePaint().setColor(Color.TRANSPARENT);
		bioimpedancePlot.setRangeLabel("Data");
		bioimpedancePlot.setTicksPerRangeLabel(3);
		bioimpedancePlot.getRangeLabelWidget().pack();

		// Format legend

		bioimpedancePlot.getLegendWidget().getTextPaint().setColor(Color.parseColor("#006bb2"));
		bioimpedancePlot.getLegendWidget().getTextPaint().setTextSize(20);
		bioimpedancePlot.getLegendWidget().setPaddingBottom(10);

		// Add series
		bioimpedancePlot.addSeries(accelXseries, new LineAndPointFormatter(Color.parseColor("#008b8b"), null, null, null));
		bioimpedancePlot.addSeries(accelYseries, new LineAndPointFormatter(Color.parseColor("#8b008b"), null, null, null));
		bioimpedancePlot.addSeries(accelZseries, new LineAndPointFormatter(Color.parseColor("#8b8b00"), null, null, null));
		bioimpedancePlot.addSeries(bioimpedanceSeries, new LineAndPointFormatter(Color.parseColor("#006bb2"), null, null, null));

		bioimpedancePlot.getBackgroundPaint().set(bgPaint);
	}

}
