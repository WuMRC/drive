package com.shemanigans.mime;

import java.io.PrintWriter;
import java.io.StringWriter;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.support.v4.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;

@SuppressLint("InflateParams")
public class FrequencySweepFragment extends DialogFragment{

	public boolean selection = false;
	public String[] value = {"0", "0", "0"};
	EditText startFreq;
	EditText stepSize;
	EditText numOfIncrements;
	private final static String TAG = FrequencySweepFragment.class.getSimpleName();

	@Override
	public Dialog onCreateDialog(Bundle savedInstanceState) {
		AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
		// Set the dialog title
		builder.setTitle(R.string.set_frequency_sweep);
		// Get the layout inflater
		LayoutInflater inflater = getActivity().getLayoutInflater();
		View view = inflater.inflate(R.layout.set_frequency_sweep, null);

		startFreq = (EditText) view.findViewById(R.id.set_upper_freq);
		stepSize = (EditText) view.findViewById(R.id.set_step_size);
		numOfIncrements = (EditText) view.findViewById(R.id.set_lower_freq);	

		try { // For activity long term, arguments may have not been set.
			value[0] = getArguments().getString(DeviceControlActivity.EXTRA_START_FREQ_BINDER);
			value[1] = getArguments().getString(DeviceControlActivity.EXTRA_STEP_SIZE_BINDER);
			value[2] = getArguments().getString(DeviceControlActivity.EXTRA_NUM_OF_INCREMENTS_BINDER);
		}
		catch (Exception e) {
			value[0] = "0";
			value[1] = "0";
			value[2] = "0";
			Log.i(TAG, getStackTrace(e));
		}
		if(Integer.parseInt(value[0]) != 0) {
			startFreq.setText(value[0]);
			stepSize.setText(value[1]);
			numOfIncrements.setText(value[2]);
		}

		// Inflate and set the layout for the dialog
		// Pass null as the parent view because its going in the dialog layout

		builder.setView(view)
		// Add action buttons
		.setPositiveButton(R.string.multi_freq, new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int id) {
				// Get new value for sample rate

				value[0] = startFreq.getText().toString();
				value[1] = stepSize.getText().toString();
				value[2] = numOfIncrements.getText().toString();			
				selection = true;
				mListener.onDialogPositiveClickFrequencySweep(FrequencySweepFragment.this);
			}
		})
		.setNeutralButton(R.string.cancel, new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int id) {			
				selection = true;
				mListener.onDialogNeutralClickFrequencySweep(FrequencySweepFragment.this);
			}
		})
		.setNegativeButton(R.string.single_freq, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int id) {
				value[0] = startFreq.getText().toString();
				value[1] = "0";
				value[2] = "0";
				mListener.onDialogNegativeClickFrequencySweep(FrequencySweepFragment.this);
				selection = false;
				FrequencySweepFragment.this.getDialog().cancel();
			}
		});      
		return builder.create();
	}

	public interface FrequencySweepListener {
		public void onDialogPositiveClickFrequencySweep(DialogFragment dialog);
		public void onDialogNegativeClickFrequencySweep(DialogFragment dialog);
		public void onDialogNeutralClickFrequencySweep(DialogFragment dialog);
	}

	// Use this instance of the interface to deliver action events
	FrequencySweepListener mListener;

	// Override the Fragment.onAttach() method to instantiate the NoticeDialogListener
	@Override
	public void onAttach(Activity activity) {
		super.onAttach(activity);
		// Verify that the host activity implements the callback interface
		try {
			// Instantiate the NoticeDialogListener so we can send events to the host
			mListener = (FrequencySweepListener) activity;
		} catch (ClassCastException e) {
			// The activity doesn't implement the interface, throw exception
			throw new ClassCastException(activity.toString()
					+ " must implement SampleRateListener");
		}
	}

	public boolean getSelection() {
		return this.selection;
	}

	public String[] getValue() {
		return this.value;
	}
	
	public static String getStackTrace(final Throwable throwable) {
	     final StringWriter sw = new StringWriter();
	     final PrintWriter pw = new PrintWriter(sw, true);
	     throwable.printStackTrace(pw);
	     return sw.getBuffer().toString();
	}
}
