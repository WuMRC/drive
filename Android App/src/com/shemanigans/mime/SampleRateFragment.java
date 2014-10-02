package com.shemanigans.mime;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;

@SuppressLint("InflateParams")
public class SampleRateFragment extends DialogFragment {

	public boolean selection = false;
	public String value;
	EditText editText;

	@Override
	public Dialog onCreateDialog(Bundle savedInstanceState) {
		AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
		// Set the dialog title
		builder.setTitle(R.string.set_sample_rate);
		// Get the layout inflater
		LayoutInflater inflater = getActivity().getLayoutInflater();
		View view = inflater.inflate(R.layout.set_sample_rate, null);

		editText = (EditText) view.findViewById(R.id.set_sample_rate);
		value = editText.getText().toString();


		// Inflate and set the layout for the dialog
		// Pass null as the parent view because its going in the dialog layout

		builder.setView(view)
		// Add action buttons
		.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int id) {
				// Get new value for sample rate
				
				value = editText.getText().toString();
				selection = true;
				
				mListener.onDialogPositiveClickSampleRate(SampleRateFragment.this);
			}
		})
		.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int id) {
				mListener.onDialogNegativeClickSampleRate(SampleRateFragment.this);
				selection = false;
				SampleRateFragment.this.getDialog().cancel();
			}
		});      
		return builder.create();
	}

	public interface SampleRateListener {
		public void onDialogPositiveClickSampleRate(DialogFragment dialog);
		public void onDialogNegativeClickSampleRate(DialogFragment dialog);
	}

	// Use this instance of the interface to deliver action events
	SampleRateListener mListener;

	// Override the Fragment.onAttach() method to instantiate the NoticeDialogListener
	@Override
	public void onAttach(Activity activity) {
		super.onAttach(activity);
		// Verify that the host activity implements the callback interface
		try {
			// Instantiate the NoticeDialogListener so we can send events to the host
			mListener = (SampleRateListener) activity;
		} catch (ClassCastException e) {
			// The activity doesn't implement the interface, throw exception
			throw new ClassCastException(activity.toString()
					+ " must implement SampleRateListener");
		}
	}

	public boolean getSelection() {
		return this.selection;
	}

	public String getValue() {
		return this.value;
	}

}
