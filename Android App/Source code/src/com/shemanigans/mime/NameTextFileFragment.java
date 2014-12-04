package com.shemanigans.mime;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.support.v4.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;

@SuppressLint("InflateParams")
public class NameTextFileFragment extends DialogFragment {

	public boolean selection = false;
	public String name;
	EditText editText;


	@Override
	public Dialog onCreateDialog(Bundle savedInstanceState) {
		AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
		// Set the dialog title
		builder.setTitle(R.string.name_text_file);
		// Get the layout inflater
		LayoutInflater inflater = getActivity().getLayoutInflater();
		View view = inflater.inflate(R.layout.set_text_file_name, null);

		editText = (EditText) view.findViewById(R.id.textfile_name);
		name = editText.getText().toString();


		// Inflate and set the layout for the dialog
		// Pass null as the parent view because its going in the dialog layout

		builder.setView(view)
		// Add action buttons
		.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int id) {
				// Get new string for file name
                mListener.onDialogPositiveClickNameTextFile(NameTextFileFragment.this);

				name = editText.getText().toString();
				selection = true;
			}
		})
		.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int id) {
                mListener.onDialogNegativeClickNameTextFile(NameTextFileFragment.this);
				selection = false;
				NameTextFileFragment.this.getDialog().cancel();
			}
		});      
		return builder.create();
	}

	public interface NameTextFileListener {
		public void onDialogPositiveClickNameTextFile(DialogFragment dialog);
		public void onDialogNegativeClickNameTextFile(DialogFragment dialog);
	}

	// Use this instance of the interface to deliver action events
	NameTextFileListener mListener;

	// Override the Fragment.onAttach() method to instantiate the NoticeDialogListener
	@Override
	public void onAttach(Activity activity) {
		super.onAttach(activity);
		// Verify that the host activity implements the callback interface
		try {
			// Instantiate the NoticeDialogListener so we can send events to the host
			mListener = (NameTextFileListener) activity;
			
		} catch (ClassCastException e) {
			// The activity doesn't implement the interface, throw exception
			throw new ClassCastException(activity.toString()
					+ " must implement NameTextFileListener");
		}
	}

	public boolean getSelection() {
		return this.selection;
	}

	public String getName() {
		return this.name;
	}

}
