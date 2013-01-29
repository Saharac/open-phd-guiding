/*
 *  stepguider.h
 *  PHD Guiding
 *
 *  Created by Bret McKee
 *  Copyright (c) 2013 Bret McKee
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
 *    Neither the name of Bret McKee, Dad Dog Development, nor the names of its
 *     Craig Stark, Stark Labs nor the names of its
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

#ifndef STEPGUIDER_H_INCLUDED
#define STEPGUIDER_H_INCLUDED

class StepGuider:public Mount
{
    int m_pCalibrationSteps;
    int m_maxDecSteps;
    int m_maxRaSteps;

    // Things related to the Advanced Config Dialog
protected:
    class StepGuiderConfigDialogPane : public MountConfigDialogPane
    {
        StepGuider *m_pStepGuider;
        wxSpinCtrl *m_pCalibrationSteps;

        public:
        StepGuiderConfigDialogPane(wxWindow *pParent, StepGuider *pStepGuider);
        ~StepGuiderConfigDialogPane(void);

        virtual void LoadValues(void);
        virtual void UnloadValues(void);
    };

    virtual int GetCalibrationSteps(void);
    virtual bool SetCalibrationSteps(int calibrationSteps);

    friend class GraphLogWindow;

public:
    virtual ConfigDialogPane *GetConfigDialogPane(wxWindow *pParent);

    // functions with an implemenation in Scope that cannot be over-ridden
    // by a subclass
public:
    StepGuider(void);
    virtual ~StepGuider(void);

private:
    bool Move(GUIDE_DIRECTION direction);
    double Move(GUIDE_DIRECTION direction, double duration);
    double CalibrationTime(int nCalibrationSteps);
    bool BacklashClearingFailed(void);

// these MUST be supplied by a subclass
private:
    virtual bool Center(void)=0;
    virtual bool Step(GUIDE_DIRECTION direction, int steps)=0;
    virtual int ApproximateStepsRemaining(GUIDE_DIRECTION direction)=0;
    virtual int ApproximateMaxStepsFromCenter(GUIDE_DIRECTION direction)=0;
    virtual bool IsAtLimit(GUIDE_DIRECTION direction, bool& atLimit) = 0;
};

#endif /* STEPGUIDER_H_INCLUDED */
