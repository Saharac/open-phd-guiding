/*
 *  frame_events.cpp
 *  PHD Guiding
 *
 *  Created by Craig Stark.
 *  Copyright (c) 2006-2010 Craig Stark.
 *  All rights reserved.
 *
 *  This source code is distributed under the following "BSD" license
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are met:
 *    Redistributions of source code must retain the above copyright notice, 
 *     this list of conditions and the following disclaimer.
 *    Redistributions in binary form must reproduce the above copyright notice, 
 *     this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *    Neither the name of Craig Stark, Stark Labs nor the names of its 
 *     contributors may be used to endorse or promote products derived from 
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "phd.h"
#include <wx/spinctrl.h>
#include <wx/textfile.h>
#include "image_math.h"
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>

// Some specific camera includes
#if defined (__WINDOWS__) && defined (LE_PARALLEL_CAMERA)
#include "cam_LEwebcam.h"
extern Camera_LEwebcamClass Camera_LEwebcamParallel;
extern Camera_LEwebcamClass Camera_LEwebcamLXUSB;
#endif

double MyFrame::RequestedExposureDuration() { // returns the duration based on pull-down
	wxString durtext;
	double dReturn;
//	if (CaptureActive) return;  // Looping an exposure already

	durtext = pFrame->Dur_Choice->GetStringSelection();
	durtext = durtext.BeforeFirst(' '); // remove the " s" bit
#if wxUSE_XLOCALE
	durtext.ToCDouble(&dReturn);
#else
	durtext.ToDouble(&dReturn);
#endif
    dReturn *= 1000;
	if (pCamera->HaveDark) {
		if (pCamera->DarkDur != dReturn)
			Dark_Button->SetBackgroundColour(wxColor(255,0,0));
		else
			Dark_Button->SetBackgroundColour(wxNullColour);
	}

    return dReturn;
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event)) {
	if (CaptureActive) return;  // Looping an exposure already
    Close(true);
}

void MyFrame::OnInstructions(wxCommandEvent& WXUNUSED(event)) {
	if (CaptureActive) return;  // Looping an exposure already
	wxMessageBox(wxString::Format(_T("Welcome to PHD (Push Here Dummy) Guiding\n\n \
Operation is quite simple (hence the 'PHD')\n\n \
  1) Press the Camera Button and select your camera\n \
  2) Select your scope interface in the Mount menu if not\n \
     already selected.  Then, press the Telescope Button \n \
     to connect to your scope\n \
  3) Pick an exposure duration from the drop-down list\n \
  4) Hit the Loop Button, adjust your focus\n \
  5) Click on a star away from the edge\n \
  6) Press the PHD (archery target) icon\n\n \
PHD will then calibrate itself and begin guiding.  That's it!\n\n \
To stop guiding, simply press the Stop Button. If you need to \n \
tweak any options, click on the Brain Button to bring up the\n \
Advanced panel.  ")),_T("Instructions"));

}

void MyFrame::OnHelp(wxCommandEvent& WXUNUSED(event)) {
	help->Display(_T("Introduction"));
}
void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event)) {
	if (CaptureActive) return;  // Looping an exposure already
#ifdef ORION
	wxMessageBox(wxString::Format(_T("PHD Guiding for Orion v%s\n\nCopyright 2006-2012 Craig Stark, Stark Labs"),VERSION),_T("About PHD Guiding"), wxOK);
#else
	wxMessageBox(wxString::Format(_T("PHD Guiding v%s\n\nwww.stark-labs.com\n\nCopyright 2006-2011 Craig Stark\n\nSpecial Thanks to:\n  Sean Prange\n  Bret McKee\n  Jared Wellman"),VERSION),_T("About PHD Guiding"), wxOK);
#endif
}

void MyFrame::OnOverlay(wxCommandEvent &evt) {
	pGuider->SetOverlayMode(evt.GetId() - MENU_XHAIR0);
}

void MyFrame::OnSave(wxCommandEvent& WXUNUSED(event)) {
	if (CaptureActive) return;  // Looping an exposure already
	wxString fname = wxFileSelector( wxT("Save FITS Image"), (const wxChar *)NULL,
                          (const wxChar *)NULL,
                           wxT("fit"), wxT("FITS files (*.fit)|*.fit"),wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (fname.IsEmpty())
        return;  // Check for canceled dialog
	if (wxFileExists(fname))
		fname = _T("!") + fname;

    if (pGuider->SaveCurrentImage(fname))
    {
        (void) wxMessageBox(_T("Error"),wxT("Your data were not saved"),wxOK | wxICON_ERROR);
    }
}

void MyFrame::OnLoadSaveDark(wxCommandEvent &evt) {
	wxString fname;

	if (evt.GetId() == MENU_SAVEDARK) {
		if (!pCamera || !pCamera->HaveDark) {
			wxMessageBox("You haven't captured a dark frame - nothing to save");
			return;
		}
		fname = wxFileSelector( wxT("Save dark (FITS Image)"), (const wxChar *)NULL,
										(const wxChar *)NULL,
										wxT("fit"), wxT("FITS files (*.fit)|*.fit"),wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (fname.IsEmpty()) return;  // Check for canceled dialog
		if (!fname.EndsWith(_T(".fit"))) fname.Append(_T(".fit"));
		if (wxFileExists(fname))
			fname = _T("!") + fname;
		if (pCamera->CurrentDarkFrame.Save(fname))
        {
		    wxMessageBox (_T("Error saving FITS file"));
        }
    }
	else if (evt.GetId() == MENU_LOADDARK) {
		if (!pCamera || !pCamera->Connected)
        {
			wxMessageBox("You must connect a camera before loading a dark");
			return;
        }
		fname = wxFileSelector( wxT("Load dark (FITS Image)"), (const wxChar *)NULL,
							   (const wxChar *)NULL,
							   wxT("fit"), wxT("FITS files (*.fit)|*.fit"), wxFD_OPEN | wxFD_CHANGE_DIR);
		if (fname.IsEmpty()) return;  // Check for canceled dialog
    
        if (pCamera->CurrentDarkFrame.Load(fname))
        {
			SetStatusText("Dark not loaded");
        }
        else
        {
			pCamera->HaveDark = true;
			tools_menu->FindItem(MENU_CLEARDARK)->Enable(pCamera->HaveDark);
			Dark_Button->SetLabel(_T("Redo Dark"));
			SetStatusText("Dark loaded");
        }
	}
}
void MyFrame::OnIdle(wxIdleEvent& WXUNUSED(event)) {
/*	if (ASCOM_IsMoving())
		SetStatusText(_T("Moving"),2);
	else
		SetStatusText(_T("Still"),2);*/
}

void MyFrame::OnLoopExposure(wxCommandEvent& WXUNUSED(event)) 
{
    try
    {
        if (!pCamera || !pCamera->Connected)
        {
            wxMessageBox(_T("Please connect to a camera first"),_T("Info"));
            throw ERROR_INFO("Camera not connected");
        }

        assert(!CaptureActive);

        pFrame->StartCapturing();

    }
    catch (wxString Msg)
    {
        POSSIBLY_UNUSED(Msg);
    }
}

/*
 * OnExposeComplete is the dispatch routine that is called when an image has been taken
 * by the background thread.
 *
 * It:
 * - causes the image to be redrawn by calling pGuider->UpateImageDisplay()
 * - calls the routine to update the guider state (which may do nothing)
 * - calls any other appropriate state update routine depending upon the current state
 * - updates button state based on appropriate state variables
 * - schedules another exposure if CaptureActive is stil true
 *
 */
void MyFrame::OnExposeComplete(wxThreadEvent& event)
{
    try
    {
        Debug.Write("Processing an image\n");

        usImage *pNewFrame = event.GetPayload<usImage *>();
        
        if (event.GetInt())
        {
            delete pNewFrame;

            StopCapturing();
            pGuider->Reset();

            Debug.Write("OnExposureComplete(): Capture Error reported\n");

            throw ERROR_INFO("Error reported capturing image");
        }

        pGuider->UpdateGuideState(pNewFrame, !CaptureActive);
        pNewFrame = NULL; // the guider owns in now

#ifdef BRET_DODO
        if (RandomMotionMode && pGuider->GetState() < STATE_CALIBRATING)
        {
			GUIDE_DIRECTION dir;
            
            if (rand() % 2)
                dir = EAST;
            else
                dir = WEST;
			int dur = rand() % 1000;
            ScheduleGuide(dir, dur, wxString::Format(_T("Random motion: %d %d"),dir,dur));

			if ((rand() % 5) == 0) {  // Occasional Dec
                if (rand() % 2)
                    dir = NORTH;
                else
                    dir = SOUTH;
				dur = rand() % 1000;
				pMount->Guide(dir,dur);
                ScheduleGuide(dir, dur, wxString::Format(_T("Random motion: %d %d"),dir,dur));
			}
        }
#endif
        
        if (CaptureActive)
        {
            ScheduleExposure(RequestedExposureDuration(), pGuider->GetBoundingBox());
        }
    }
    catch (wxString Msg)
    {
        POSSIBLY_UNUSED(Msg);
    }
}

void MyFrame::OnMoveComplete(wxThreadEvent& event)
{
    try
    {
        if (event.GetInt())
        {
            throw ERROR_INFO("Error reported moving");
        }
    }
    catch (wxString Msg)
    {
        POSSIBLY_UNUSED(Msg);
    }
}

void MyFrame::OnButtonStop(wxCommandEvent& WXUNUSED(event)) 
{
    StopCapturing();
}

void MyFrame::OnGammaSlider(wxScrollEvent& WXUNUSED(event)) {
	Stretch_gamma = (double) Gamma_Slider->GetValue() / 100.0;
	pGuider->UpdateImageDisplay();
}

void MyFrame::OnDark(wxCommandEvent& WXUNUSED(event)) {
    double ExpDur = RequestedExposureDuration();
	if (pGuider->GetState() > STATE_SELECTED) return;
	if (!pCamera || !pCamera->Connected) {
		wxMessageBox(_T("Please connect to a camera first"),_T("Info"));
		return;
	}
	if (CaptureActive) return;  // Looping an exposure already
	Dark_Button->SetForegroundColour(wxColour(200,0,0));
	int NDarks = 5;

	SetStatusText(_T("Capturing dark"));
	if (pCamera->HasShutter) 
		pCamera->ShutterState=true; // dark
	else
		wxMessageBox(_T("Cover guide scope"));
	pCamera->InitCapture();
  	if (pCamera->Capture(ExpDur, pCamera->CurrentDarkFrame, false)) {
		wxMessageBox(_T("Error capturing dark frame"));
		pCamera->HaveDark = false;
		SetStatusText(wxString::Format(_T("%.1f s dark FAILED"),(float) ExpDur / 1000.0));
		Dark_Button->SetLabel(_T("Take Dark"));
		pCamera->ShutterState=false;
	}
	else {
		SetStatusText(wxString::Format(_T("%.1f s dark #1 captured"),(float) ExpDur / 1000.0));
		int *avgimg = new int[pCamera->CurrentDarkFrame.NPixels];
		int i, j;
		int *iptr = avgimg;
		unsigned short *usptr = pCamera->CurrentDarkFrame.ImageData;
		for (i=0; i<pCamera->CurrentDarkFrame.NPixels; i++, iptr++, usptr++)
			*iptr = (int) *usptr;
		for (j=1; j<NDarks; j++) {
			pCamera->Capture(ExpDur, pCamera->CurrentDarkFrame, false);
			iptr = avgimg;
			usptr = pCamera->CurrentDarkFrame.ImageData;
			for (i=0; i<pCamera->CurrentDarkFrame.NPixels; i++, iptr++, usptr++)
				*iptr = *iptr + (int) *usptr;
			SetStatusText(wxString::Format(_T("%.1f s dark #%d captured"),(float) ExpDur / 1000.0,j+1));
		}
		iptr = avgimg;
		usptr = pCamera->CurrentDarkFrame.ImageData;
		for (i=0; i<pCamera->CurrentDarkFrame.NPixels; i++, iptr++, usptr++)
			*usptr = (unsigned short) (*iptr / NDarks);


		Dark_Button->SetLabel(_T("Redo Dark"));
		pCamera->HaveDark = true;
		pCamera->DarkDur = ExpDur;
	}
	SetStatusText(_T("Darks done"));
	if (pCamera->HasShutter)
		pCamera->ShutterState=false; // Lights
	else 
		wxMessageBox(_T("Uncover guide scope"));
	tools_menu->FindItem(MENU_CLEARDARK)->Enable(pCamera->HaveDark);
}

void MyFrame::OnClearDark(wxCommandEvent& WXUNUSED(evt)) {
	if (!pCamera->HaveDark) return;
	Dark_Button->SetLabel(_T("Take Dark"));
	Dark_Button->SetForegroundColour(wxColour(0,0,0));
	pCamera->HaveDark = false;
	tools_menu->FindItem(MENU_CLEARDARK)->Enable(pCamera->HaveDark);
}

void MyFrame::OnGraph(wxCommandEvent &evt) {
	this->GraphLog->SetState(evt.IsChecked());
}

void MyFrame::OnStarProfile(wxCommandEvent &evt) {
	this->Profile->SetState(evt.IsChecked());
}

void MyFrame::OnLog(wxCommandEvent &evt) {
	if (evt.GetId() == MENU_LOG) {
		if (evt.IsChecked()) {  // enable it
			Log_Data = true;
			if (!LogFile->IsOpened()) {
				if (LogFile->Exists()) LogFile->Open();
				else LogFile->Create();
			}
			wxDateTime now = wxDateTime::Now();
			LogFile->AddLine(_T("Logging manually enabled"));
			LogFile->AddLine(wxString::Format(_T("PHD Guide %s  -- "),VERSION) + now.FormatDate()  + _T(" ") + now.FormatTime());
			LogFile->Write();
			this->SetTitle(wxString::Format(_T("PHD Guiding %s  -  www.stark-labs.com (Log active)"),VERSION));
		}
		else {
			if (LogFile->IsOpened()) {
				LogFile->AddLine(_T("Logging manually disabled"));
				LogFile->Write();
				LogFile->Close();
			}
			Log_Data = false;
			this->SetTitle(wxString::Format(_T("PHD Guiding %s  -  www.stark-labs.com"),VERSION));
		}
	} else if (evt.GetId() == MENU_LOGIMAGES) {
		if (wxGetKeyState(WXK_SHIFT)) {
//			wxMessageBox("arg");
#ifdef __WINDOWS__
//			tools_menu->FindItem(MENU_LOGIMAGES)->SetTextColour(wxColour(200,10,10));
#endif
			tools_menu->FindItem(MENU_LOGIMAGES)->SetItemLabel(_T("Enable Raw Star logging"));
			if (evt.IsChecked())
				Log_Images = 2;
			else
				Log_Images = 0;
		}
		else {
#ifdef __WINDOWS__
//			tools_menu->FindItem(MENU_LOGIMAGES)->SetTextColour(*wxBLACK);
#endif
			tools_menu->FindItem(MENU_LOGIMAGES)->SetText(_T("Enable Star Image logging"));
			if (evt.IsChecked())
				Log_Images = 1;
			else
				Log_Images = 0;
			
		}
		Menubar->Refresh();
	} else if (evt.GetId() == MENU_DEBUG)
    {
        Debug.SetState(evt.IsChecked()); 
    }
}

bool MyFrame::FlipRACal( wxCommandEvent& WXUNUSED(evt))
{
	return !pMount->FlipCalibration();
}

void MyFrame::OnAutoStar(wxCommandEvent& WXUNUSED(evt)) {
    pFrame->pGuider->AutoSelect();
}

#ifndef __WXGTK__
void MyFrame::OnDonateMenu(wxCommandEvent &evt) {

	switch (evt.GetId()) {
		case DONATE1:
			wxLaunchDefaultBrowser(_T("https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=11353812"));
			break;
		case DONATE2:
			wxLaunchDefaultBrowser(_T("https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=6NAB6S65UNHP4"));
			break;
		case DONATE3:
			wxLaunchDefaultBrowser(_T("https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=VHJPKAQZVF9GN"));
			break;
		case DONATE4:
			wxLaunchDefaultBrowser(_T("https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=CGUQHJLE9GR8A"));
			break;
	}

}
#endif

void MyFrame::OnSetupCamera(wxCommandEvent& WXUNUSED(event)) {
	if (!pCamera || !pCamera->Connected || !pCamera->HasPropertyDialog) return;  // One more safety check

	pCamera->ShowPropertyDialog();

}

void MyFrame::OnAdvanced(wxCommandEvent& WXUNUSED(event)) {

	if (CaptureActive) return;  // Looping an exposure already
	AdvancedDialog* dlog = new AdvancedDialog();

    dlog->LoadValues();

	if (dlog->ShowModal() == wxID_OK) 
    {
        dlog->UnloadValues();
#ifdef BRET_TODO
	pFrame->GraphLog->RAA_Ctrl->SetValue((int) (RA_aggr * 100));
	pFrame->GraphLog->RAH_Ctrl->SetValue((int) (RA_hysteresis * 100));
#if ((wxMAJOR_VERSION > 2) || (wxMINOR_VERSION > 8))
	pFrame->GraphLog->MM_Ctrl->SetValue(MinMotion);
#endif
	pFrame->GraphLog->MDD_Ctrl->SetValue(Max_Dec_Dur);
	pFrame->GraphLog->DM_Ctrl->SetSelection(Dec_guide);
#endif // BRET_TODO
    }
}

void MyFrame::OnGuide(wxCommandEvent& WXUNUSED(event)) {
    try
    {
        if (pMount == NULL) 
        {
            // no mount selected -- should never happen
            throw ERROR_INFO("pMount == NULL");
        }

        if (!pMount->IsConnected())
        {
            throw ERROR_INFO("Unable to guide with no scope Connected");
        }

        if (!pCamera || !pCamera->Connected)
        {
            throw ERROR_INFO("Unable to guide with no camera Connected");
        }

        if (pGuider->GetState() < STATE_SELECTED)
        {
            wxMessageBox(_T("Please select a guide star before attempting to guide"));
            throw ERROR_INFO("Unable to guide with state < STATE_SELECTED");
        }

        pGuider->StartGuiding();

        StartCapturing();
    }
    catch (wxString Msg)
    {
        POSSIBLY_UNUSED(Msg);
        pGuider->Reset();
    }
    return;
}

void MyFrame::OnTestGuide(wxCommandEvent& WXUNUSED(evt)) {
	if ((pFrame->pGuider->GetState() > STATE_SELECTED) || !(pMount->IsConnected())) return;
	TestGuideDialog* dlog = new TestGuideDialog();
	dlog->Show();
}

