/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
void AudioAnalyzerAudioProcessorEditor::drawPanel() {

	if (aP.cS.menuBarHide) { hidePanel(); return; }

	panel.setBounds((aP.cS.newW / 2) - (defW / 2), 0, defW, defH);

	frame.setTextLabelPosition(juce::Justification::centred);
	juce::Rectangle barBounds = { 40, 22, 720, 58 };
	frame.setBounds(barBounds);

	setbarmenu.setBounds(barBounds);
	panel.addAndMakeVisible(setbarmenu);
	frame.removeAllChildren();
	panel.addAndMakeVisible(frame);

	frame.addAndMakeVisible(specbutton);
	frame.addAndMakeVisible(sonobutton);
	//    frame.addAndMakeVisible(wavebutton);

	if (aP.cS.showSettings) { set_button.setToggleState(true, juce::dontSendNotification); }
	frame.addAndMakeVisible(set_button);

	//    frame.addAndMakeVisible(stereo_midside);

	if (aP.sImg.ch1L) { ch1Lbutton.setToggleState(true, juce::dontSendNotification); }
	if (aP.sImg.ch1R) { ch1Rbutton.setToggleState(true, juce::dontSendNotification); }
	if (aP.sImg.ch2L) { ch2Lbutton.setToggleState(true, juce::dontSendNotification); }
	if (aP.sImg.ch2R) { ch2Rbutton.setToggleState(true, juce::dontSendNotification); }
	//    if (aP.cS.ch1M) { ch1Mbutton.setToggleState(true, juce::dontSendNotification); }
	//    if (aP.cS.ch1S) { ch1Sbutton.setToggleState(true, juce::dontSendNotification); }
	//    if (aP.cS.ch2M) { specbutton.setToggleState(true, juce::dontSendNotification); }
	//    if (aP.cS.ch2S) { ch2Sbutton.setToggleState(true, juce::dontSendNotification); }

	frame.addAndMakeVisible(ch1Lbutton);
	frame.addAndMakeVisible(ch1Rbutton);
	if (curAnalyzerMode == spec) {
		frame.addAndMakeVisible(ch2Lbutton);
		frame.addAndMakeVisible(ch2Rbutton);
	}

	//    if (aP.cS.menuChSwitch) {
	//        stereo_midside.setButtonText("STEREO");
	//        frame.addAndMakeVisible(ch1Lbutton);
	//        frame.addAndMakeVisible(ch1Rbutton);
	//        frame.addAndMakeVisible(ch2Lbutton);
	//        frame.addAndMakeVisible(ch2Rbutton);
	//    } else {
	//        stereo_midside.setButtonText("MIDSIDE");
	//        frame.addAndMakeVisible(ch1Mbutton);
	//        frame.addAndMakeVisible(ch1Sbutton);
	//        frame.addAndMakeVisible(ch2Mbutton);
	//        frame.addAndMakeVisible(ch2Sbutton);
	//    }

	if (aP.cS.showSettings) { showSetPanel(); }
	frame.addAndMakeVisible(hidebutton);

}
void AudioAnalyzerAudioProcessorEditor::showSetPanel()
{
	if (aP.cS.showSettings) {
		settingsFrame.removeAllChildren();
		settingsFrame.setTextLabelPosition(juce::Justification::centred);
		juce::Rectangle barBounds = { 40, 80, 720, 240 };
		settingsFrame.setBounds(barBounds);
		setFrameBG.setBounds(barBounds);
		panel.addAndMakeVisible(setFrameBG);
		panel.addAndMakeVisible(settingsFrame);

		settingsFrame.addAndMakeVisible(gainSlider);

		if (curAnalyzerMode == spec || curAnalyzerMode == sono) {
			settingsFrame.addAndMakeVisible(slopeSlider);

			settingsFrame.addAndMakeVisible(floorSlider);

			if (aP.cS.setLiner == true) { setlinerbutton.setToggleState(true, juce::dontSendNotification); }
			settingsFrame.addAndMakeVisible(setlinerbutton);

			//            if (aP.cS.setNorm == true) { setNormbutton.setToggleState(true, juce::dontSendNotification); }
			//            settingsFrame.addAndMakeVisible(setNormbutton);

						//** WindowMethod GROUP ======================
			settingsFrame.addAndMakeVisible(setWinGroup);

			setWinGroup.addAndMakeVisible(fftWin1button);
			setWinGroup.addAndMakeVisible(fftWin2button);
			setWinGroup.addAndMakeVisible(fftWin3button);
			setWinGroup.addAndMakeVisible(fftWin4button);
			setWinGroup.addAndMakeVisible(fftWin5button);

			if (aP.cS.winMet == juce::dsp::WindowingFunction<float>::hann)
			{
				fftWin1button.setToggleState(true, juce::dontSendNotification);
			}

			if (aP.cS.winMet == juce::dsp::WindowingFunction<float>::hamming)
			{
				fftWin2button.setToggleState(true, juce::dontSendNotification);
			}

			if (aP.cS.winMet == juce::dsp::WindowingFunction<float>::blackman)
			{
				fftWin3button.setToggleState(true, juce::dontSendNotification);
			}

			if (aP.cS.winMet == juce::dsp::WindowingFunction<float>::blackmanHarris)
			{
				fftWin4button.setToggleState(true, juce::dontSendNotification);
			}

			if (aP.cS.winMet == juce::dsp::WindowingFunction<float>::flatTop)
			{
				fftWin5button.setToggleState(true, juce::dontSendNotification);
			}

			//** FFT Size GROUP ==========================
			settingsFrame.addAndMakeVisible(setFftGroup);

			if (*aP.cS.fftOrder == 12) { fft12_button.setToggleState(true, juce::dontSendNotification); }
			if (*aP.cS.fftOrder == 13) { fft13_button.setToggleState(true, juce::dontSendNotification); }
			if (*aP.cS.fftOrder == 14) { fft14_button.setToggleState(true, juce::dontSendNotification); }

			//** FFT Size GROUP ==========================
			settingsFrame.addAndMakeVisible(setOverlapGroup);
			if (*aP.cS.overlap == 2) { overlap2button.setToggleState(true, juce::dontSendNotification); }
			if (*aP.cS.overlap == 4) { overlap4button.setToggleState(true, juce::dontSendNotification); }
			if (*aP.cS.overlap == 8) { overlap8button.setToggleState(true, juce::dontSendNotification); }
			if (*aP.cS.overlap == 16) { overlap16button.setToggleState(true, juce::dontSendNotification); }

		}

		if (curAnalyzerMode == spec) {
			settingsFrame.addAndMakeVisible(crLineSlider);
		}

		if (curAnalyzerMode == sono) {
			settingsFrame.addAndMakeVisible(colorLSlider);
			settingsFrame.addAndMakeVisible(colorRSlider);
			settingsFrame.addAndMakeVisible(saturatSlider);
		}

		settingsFrame.addAndMakeVisible(curVersionLabel);
		settingsFrame.addAndMakeVisible(urlVersionButton);
	}
}
void AudioAnalyzerAudioProcessorEditor::repaintPanel()
{
	showbutton.setVisible(false);
	settingsFrame.removeAllChildren();
	frame.removeAllChildren();
	panel.removeAllChildren();
	drawPanel();
}
void AudioAnalyzerAudioProcessorEditor::hidePanel()
{
	settingsFrame.removeAllChildren();
	panel.removeAllChildren();
	frame.removeAllChildren();
	panel.setBounds(getWidth(), 0, 1, 1);
	showbutton.setVisible(true);
}

//==============================================================================
AudioAnalyzerAudioProcessorEditor::AudioAnalyzerAudioProcessorEditor(AudioAnalyzerAudioProcessor& p)
	: AudioProcessorEditor(&p),
	aP(p)
{
	int curW = aP.cS.newW = (aP.cS.newW) ? aP.cS.newW : defW;
	int curH = aP.cS.newH = (aP.cS.newH) ? aP.cS.newH : defH;

	//sonogramImage = new juce::Image(juce::Image::ARGB, curW, curH, true);

	setLookAndFeel(&otherLookAndFeel);

	//    curAnalyzerMode = (curAnalyzerMode) ? curAnalyzerMode : sono;
	switch (aP.cS.mode) {
	case 1:
		curAnalyzerMode = spec;
		break;
	case 2:
		curAnalyzerMode = sono;
		break;
	default:
		curAnalyzerMode = spec;
		break;
	}

	//    curFftSize = fft4096;

	freqLabel.setBounds(10, 0, 80, 30);
	freqLabel.setFont(juce::Font(16.0f, juce::Font::bold));
	freqLabel.setColour(juce::Label::textColourId, juce::Colours::black);
	freqLabel.setColour(juce::Label::backgroundColourId, juce::Colours::grey);
	addChildComponent(freqLabel);

	addAndMakeVisible(panel);

	addMouseListener(this, true);

	specbutton.setButtonText("SPEC");
	sonobutton.setButtonText("SONO");
	//    wavebutton.setButtonText("WAVE");

	specbutton.setBounds(10, 16, 80, 30);
	sonobutton.setBounds(95, 16, 80, 30);
	//    wavebutton.setBounds(180, 16, 80, 30);

	specbutton.setRadioGroupId(ModeButtons);
	sonobutton.setRadioGroupId(ModeButtons);
	//    wavebutton.setRadioGroupId (ModeButtons);

	specbutton.setClickingTogglesState(true);
	sonobutton.setClickingTogglesState(true);
	//    wavebutton.setClickingTogglesState (true);
	if (curAnalyzerMode == spec) { specbutton.setToggleState(true, juce::dontSendNotification); }
	if (curAnalyzerMode == sono) { sonobutton.setToggleState(true, juce::dontSendNotification); }
	//    if (curAnalyzerMode == wave) { wavebutton.setToggleState(true, juce::dontSendNotification); }
	specbutton.onClick = [this] {
		aP.cS.mode = curAnalyzerMode = spec;
		aP.cS.fftOrder = &aP.cS.fftOrderSpec;
		aP.cS.overlap = &aP.cS.overlapSpec;
		createFreqGrid();
		drawPanel();
		};
	sonobutton.onClick = [this] {
		aP.cS.mode = curAnalyzerMode = sono;
		aP.cS.fftOrder = &aP.cS.fftOrderSono;
		aP.cS.overlap = &aP.cS.overlapSono;
		createFreqGrid();
		drawPanel();
		};
	//    wavebutton.onClick = [this] { curAnalyzerMode = wave; showSetPanel(); };

		//** SETUP BUTTON =========================
	set_button.setButtonText("SET");
	set_button.setBounds(360, 16, 60, 30);
	set_button.setClickingTogglesState(true);
	set_button.onClick = [this] { aP.cS.showSettings = (aP.cS.showSettings) ? false : true; repaintPanel(); };

	//** STEREO MIDSIDE =======================
	ch1Lbutton.setButtonText("L1");
	ch1Rbutton.setButtonText("R1");
	ch2Lbutton.setButtonText("L2");
	ch2Rbutton.setButtonText("R2");

	//    ch1Mbutton.setButtonText("M1");
	//    ch1Sbutton.setButtonText("S1");
	//    ch2Mbutton.setButtonText("M2");
	//    ch2Sbutton.setButtonText("S2");

	//    stereo_midside.setBounds(425, 16, 90, 30);
	ch1Lbutton.setBounds(515, 16, 40, 30);
	ch1Rbutton.setBounds(555, 16, 40, 30);
	ch2Lbutton.setBounds(595, 16, 40, 30);
	ch2Rbutton.setBounds(635, 16, 40, 30);
	//    ch1Mbutton.setBounds(515, 16, 40, 30);
	//    ch1Sbutton.setBounds(555, 16, 40, 30);
	//    ch2Mbutton.setBounds(595, 16, 40, 30);
	//    ch2Sbutton.setBounds(635, 16, 40, 30);

	ch1Lbutton.setColour(juce::TextButton::textColourOffId, juce::Colours::darkgrey);
	ch1Rbutton.setColour(juce::TextButton::textColourOffId, juce::Colours::darkgrey);
	ch2Lbutton.setColour(juce::TextButton::textColourOffId, juce::Colours::darkgrey);
	ch2Rbutton.setColour(juce::TextButton::textColourOffId, juce::Colours::darkgrey);

	//    ch1Mbutton.setColour(juce::TextButton::textColourOffId, juce::Colours::grey);
	//    ch1Sbutton.setColour(juce::TextButton::textColourOffId, juce::Colours::grey);
	//    ch2Mbutton.setColour(juce::TextButton::textColourOffId, juce::Colours::grey);
	//    ch2Sbutton.setColour(juce::TextButton::textColourOffId, juce::Colours::grey);

	ch1Lbutton.setClickingTogglesState(true);
	ch1Rbutton.setClickingTogglesState(true);
	ch2Lbutton.setClickingTogglesState(true);
	ch2Rbutton.setClickingTogglesState(true);
	//    ch1Mbutton.setClickingTogglesState (true);
	//    ch1Sbutton.setClickingTogglesState (true);
	//    ch2Mbutton.setClickingTogglesState (true);
	//    ch2Sbutton.setClickingTogglesState (true);

	ch1Lbutton.onClick = [this] { aP.sImg.ch1L = (aP.sImg.ch1L) ? false : true; };
	ch1Rbutton.onClick = [this] { aP.sImg.ch1R = (aP.sImg.ch1R) ? false : true; };
	ch2Lbutton.onClick = [this] { aP.sImg.ch2L = (aP.sImg.ch2L) ? false : true; };
	ch2Rbutton.onClick = [this] { aP.sImg.ch2R = (aP.sImg.ch2R) ? false : true; };

	//    stereo_midside.onClick = [this] {
	//        aP.cS.menuChSwitch = (aP.cS.menuChSwitch) ? false : true; repaintPanel(); };

		// HIDE BUTTON
	hidebutton.setBounds(680, 16, 30, 30);
	hidebutton.setButtonText(">");
	hidebutton.onClick = [this] { aP.cS.menuBarHide = true; hidePanel(); };

	showbutton.setBounds(getWidth() - 30, 30, 30, 30);
	showbutton.setButtonText("<");
	addChildComponent(showbutton);
	showbutton.onClick = [this] { aP.cS.menuBarHide = false; repaintPanel(); };

	gainSlider.setParamFloat(-24.0f, 24.0f, defV.gain, 0.1f, aP.cS.gain);
	slopeSlider.setParamFloat(0.0f, 6.0f, defV.slope, 0.1f, aP.cS.slope);
	floorSlider.setParamFloat(-120.0f, -30.0f, defV.floor, 10.0f, aP.cS.floor);
	crLineSlider.setParamFloat(0.0f, 30.0f, defV.lineCR, 5.0f, aP.sImg.lineCR);
	colorLSlider.setParamFloat(0.0f, 360.0f, defV.colorSonoL, 1.0f, aP.sImg.colorSonoL);
	colorRSlider.setParamFloat(0.0f, 360.0f, defV.colorSonoR, 1.0f, aP.sImg.colorSonoR);
	saturatSlider.setParamFloat(0.0f, 1.0f, defV.saturatSono, 0.1f, aP.sImg.saturatSono);

	setlinerbutton.setBounds(260, 25, 60, 20);
	setlinerbutton.setClickingTogglesState(true);
	setlinerbutton.onClick = [this] {
		aP.cS.setLiner = (aP.cS.setLiner) ? false : true; createFreqGrid(); };

	//    setNormbutton.setBounds(260, 50, 60, 20);
	//    setNormbutton.setClickingTogglesState (true);
	//    setNormbutton.onClick = [this] {
	//        aP.cS.setNorm = (aP.cS.setNorm) ? false : true; };

		//** OVERLAP BUTTONS GROUP ==========================
	overlap2button.setRadioGroupId(overlapButtons);
	overlap4button.setRadioGroupId(overlapButtons);
	overlap8button.setRadioGroupId(overlapButtons);
	overlap16button.setRadioGroupId(overlapButtons);

	setOverlapGroup.setText("Overlap");
	setOverlapGroup.setBounds(280, 180, 242, 45);

	overlap2button.setBounds(8, 17, 50, 20);
	overlap4button.setBounds(60, 17, 50, 20);
	overlap8button.setBounds(112, 17, 60, 20);
	overlap16button.setBounds(174, 17, 60, 20);

	overlap2button.setClickingTogglesState(true);
	overlap4button.setClickingTogglesState(true);
	overlap8button.setClickingTogglesState(true);
	overlap16button.setClickingTogglesState(true);

	setOverlapGroup.addAndMakeVisible(overlap2button);
	setOverlapGroup.addAndMakeVisible(overlap4button);
	setOverlapGroup.addAndMakeVisible(overlap8button);
	setOverlapGroup.addAndMakeVisible(overlap16button);

	overlap2button.onClick = [this] { *aP.cS.overlap = 2; };
	overlap4button.onClick = [this] { *aP.cS.overlap = 4; };
	overlap8button.onClick = [this] { *aP.cS.overlap = 8; };
	overlap16button.onClick = [this] { *aP.cS.overlap = 16; };

	//** WindowMethod GROUP ======================
	fftWin1button.setRadioGroupId(fftSizeWindow);
	fftWin2button.setRadioGroupId(fftSizeWindow);
	fftWin3button.setRadioGroupId(fftSizeWindow);
	fftWin4button.setRadioGroupId(fftSizeWindow);
	fftWin5button.setRadioGroupId(fftSizeWindow);

	setWinGroup.setText("FFT Window");
	setWinGroup.setBounds(10, 130, 526, 45);

	fftWin1button.setBounds(8, 17, 60, 20);
	fftWin2button.setBounds(70, 17, 100, 20);
	fftWin3button.setBounds(172, 17, 100, 20);
	fftWin4button.setBounds(274, 17, 160, 20);
	fftWin5button.setBounds(436, 17, 80, 20);

	fftWin1button.setClickingTogglesState(true);
	fftWin2button.setClickingTogglesState(true);
	fftWin3button.setClickingTogglesState(true);
	fftWin4button.setClickingTogglesState(true);
	fftWin5button.setClickingTogglesState(true);

	fftWin1button.onClick = [this] {
		aP.cS.winMet = juce::dsp::WindowingFunction<float>::hann; };

	fftWin2button.onClick = [this] {
		aP.cS.winMet = juce::dsp::WindowingFunction<float>::hamming; };

	fftWin3button.onClick = [this] {
		aP.cS.winMet = juce::dsp::WindowingFunction<float>::blackman; };

	fftWin4button.onClick = [this] {
		aP.cS.winMet = juce::dsp::WindowingFunction<float>::blackmanHarris; };

	fftWin5button.onClick = [this] {
		aP.cS.winMet = juce::dsp::WindowingFunction<float>::flatTop; };

	//** FFT Size GROUP ==========================
	fft12_button.setRadioGroupId(fftSizeButtons);
	fft13_button.setRadioGroupId(fftSizeButtons);
	fft14_button.setRadioGroupId(fftSizeButtons);

	setFftGroup.setText("FFT Block Size");
	setFftGroup.setBounds(10, 180, 182, 45);

	fft12_button.setBounds(8, 17, 50, 20);
	fft13_button.setBounds(60, 17, 50, 20);
	fft14_button.setBounds(112, 17, 60, 20);

	fft12_button.setClickingTogglesState(true);
	fft13_button.setClickingTogglesState(true);
	fft14_button.setClickingTogglesState(true);

	setFftGroup.addAndMakeVisible(fft12_button);
	setFftGroup.addAndMakeVisible(fft13_button);
	setFftGroup.addAndMakeVisible(fft14_button);

	fft12_button.onClick = [this] { *aP.cS.fftOrder = 12; };
	fft13_button.onClick = [this] { *aP.cS.fftOrder = 13; };
	fft14_button.onClick = [this] { *aP.cS.fftOrder = 14; };

	setSize(curW, curH);
	//    setOpaque (true);
	setResizable(true, true);
	setResizeLimits(defW, defH, 7680, 4320);
	//#ifdef JUCE_OPENGL
	//    openGLContext.attachTo(*getTopLevelComponent());
	//#endif
	startTimerHz(30);

	//    juce::String new_ver;
	//    juce::URL ver_url("https://raw.githubusercontent.com/rty65tt/AudioAnalyzer/main/Source/version.h");
	//    if (ver_url.isWellFormed())
	//    {
	//        if (auto inputStream = ver_url.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
	//            .withConnectionTimeoutMs(1000)
	//            .withNumRedirectsToFollow(0)))
	//        {
	//            new_ver = ver_url.readEntireTextStream();
	//        }
	//    }
	//    juce::String nv_trim = new_ver.trim();

	juce::String version("ver ");
	//version.append("0.3.0", 10);
	version.append(JucePlugin_VersionString, 10);

	curVersionLabel.setFont(juce::Font(14.0f, juce::Font::bold));
	curVersionLabel.setColour(juce::Label::textColourId,
		juce::Colour::fromRGB(150, 150, 150));
	curVersionLabel.setColour(juce::Label::backgroundColourId, juce::Colours::darkgrey);
	curVersionLabel.setText(version, juce::dontSendNotification);

	urlVersionButton.setButtonText("GITHUB");

	urlVersionButton.setColour(juce::TextButton::textColourOffId,
		juce::Colour::fromRGB(0, 0, 180));
	urlVersionButton.setColour(juce::TextButton::buttonOver,
		juce::Colour::fromRGB(0, 0, 180));


	//    if (nv_trim.compare(JucePlugin_VersionString)) {  // 0 if identical
	//        urlVersionButton.setButtonText("UPDATE");
	//    }
	urlVersionButton.onClick = [this] {
		juce::URL url("https://github.com/rty65tt/AudioAnalyzer");
		url.launchInDefaultBrowser();
		};

	curVersionLabel.setBounds(620, 180, 80, 20);
	urlVersionButton.setBounds(620, 200, 80, 20);

}


AudioAnalyzerAudioProcessorEditor::~AudioAnalyzerAudioProcessorEditor()
{
	setLookAndFeel(nullptr);
	removeMouseListener(this);
	//sonogramImage->~Image();
//#ifdef JUCE_OPENGL
//    openGLContext.detach();
//#endif
}

//==============================================================================
void AudioAnalyzerAudioProcessorEditor::paint(juce::Graphics& g)
{
	//g.fillAll(juce::Colours::black);
	drawFreqGrid(g); // need optimazation

	if (flagStart) { flagStart = false; return; } // pillar.crutch
	switch (aP.cS.mode) {
	case 1:
		drawSpectrogram(g);
		break;
	case 2:
		aP.sImg.drawSonogram(g);
		break;
	case wave:
		break;
	}
}

void AudioAnalyzerAudioProcessorEditor::resized()
{
	// This is generally where you'll want to lay out the positions of any
	// subcomponents in your editor..
	aP.cS.resize = true;
	plotFrame = getLocalBounds();
	aP.cS.newW = getWidth();
	aP.cS.newH = getHeight();
	aP.sImg.setSizeImg(aP.cS.newW, aP.cS.newH);
	createFreqGrid();
	drawPanel();
	showbutton.setBounds(getWidth() - 30, 30, 30, 30);
}

// =============================
void AudioAnalyzerAudioProcessorEditor::createFreqGrid() {
	const float width = getLocalBounds().getWidth();
	const float height = aP.cS.mode == spec ? getLocalBounds().getHeight() : 20;

	if (gridImage != nullptr) { gridImage->~Image(); }
	gridImage = new juce::Image(juce::Image::RGB, width, height, true);

	DBG("reDraw Freq Grid: ");
	juce::Graphics gi(*gridImage);

	if (aP.cS.mode == spec) {
		const auto gColor1 = juce::Colour::fromRGBA(12, 12, 13, 255);
		const auto gColor2 = juce::Colour::fromRGBA(22, 22, 25, 255);
		const auto gradBG = juce::ColourGradient(gColor1, width / 2, height, gColor2, width / 2, 0.0f, false);
		gi.setGradientFill(gradBG);
		gi.fillAll();
	}

	const float max = aP.getSampleRate() / 2;
	if (aP.cS.mode == spec) {
		gi.setColour(juce::Colour::fromRGBA(75, 75, 75, 255));
	}
	else {
		gi.setColour(juce::Colour::fromRGBA(125, 125, 125, 255));
	}
	float x = 0.0f;
	int k = (cS->setLiner) ? 1000 : 10;
	for (auto i = k; i < max; i = (i * 10)) {
		float y = 0.0f;
		for (auto f = i; f < (i * 10); f = (f + i)) {
			if (f > k && f < max) {
				const float position = (cS->setLiner) ? f : invLogScale(f, 10, max);
				x = width * position / max;
				if (aP.cS.mode == spec) { gi.drawLine({ round(x), height, round(x), 21.0f }); }
				if ((x - y) > 30.0f) {
					auto freq = (f < 1000) ? f : (f / 1000);
					auto a = (f < 1000) ? juce::String(freq) : juce::String(freq) + "k";
					gi.drawFittedText(a, x - 5, 3, 20, 14, juce::Justification::left, 1);
				}
				y = x;
			}
		}
	}

	if (aP.cS.mode == spec) {
		auto maxDB = aP.cS.floor * (-1);
		float s = height / maxDB * 10;
		float y = 20.0f;
		for (auto i = 0; i < maxDB; i = (i + 10)) {
			auto a = juce::String(i) + "db";
			gi.drawFittedText(a, 5, y - 7, 30, 14, juce::Justification::left, 1);
			gi.drawLine({ 40.0f, round(y), width, round(y) });
			y = y + s;
		}
	}
}

void AudioAnalyzerAudioProcessorEditor::drawFreqGrid(juce::Graphics& g) {
	g.drawImageAt(*gridImage, 0, 0, false);

}
void AudioAnalyzerAudioProcessorEditor::drawSpectrogram(juce::Graphics& g) {


	const auto inColour1L = juce::Colour::fromRGBA(75, 75, 175, 205);
	const auto inColour1R = juce::Colour::fromRGBA(46, 139, 87, 205);
	const auto inColour2L = juce::Colour::fromRGBA(175, 46, 35, 205);
	const auto inColour2R = juce::Colour::fromRGBA(139, 139, 46, 205);

	float cR = aP.sImg.lineCR;
	aP.createAnalyserPlot();
	if (aP.sImg.ch2R) {
		g.setColour(inColour2R);
		g.strokePath(aP.analyserPathCh2R.createPathWithRoundedCorners(cR), juce::PathStrokeType(2.0));
	}
	if (aP.sImg.ch2L) {
		g.setColour(inColour2L);
		g.strokePath(aP.analyserPathCh2L.createPathWithRoundedCorners(cR), juce::PathStrokeType(2.0));
	}
	if (aP.sImg.ch1R) {
		g.setColour(inColour1R);
		g.strokePath(aP.analyserPathCh1R.createPathWithRoundedCorners(cR), juce::PathStrokeType(2.0));
	}
	if (aP.sImg.ch1L) {
		g.setColour(inColour1L);
		g.strokePath(aP.analyserPathCh1L.createPathWithRoundedCorners(cR), juce::PathStrokeType(2.0));
	}

	g.setColour(juce::Colours::black);
	g.drawRect(plotFrame.toFloat(), 3);
	g.drawRoundedRectangle(plotFrame.toFloat(), 15, 3);
}

float AudioAnalyzerAudioProcessorEditor::logScale(const float value, const float min, const float max)
{
	if (value < min)
		return min;
	if (value > max)
		return max;

	const float b = std::log(max / min) / (max - min);
	const float a = max / std::exp(max * b);

	return a * std::exp(b * value);
}

float AudioAnalyzerAudioProcessorEditor::invLogScale(const float value, const float min, const float max)
{
	if (value < min) return min;
	if (value > max) return max;
	const float b = std::log(max / min) / (max - min);
	const float a = max / std::exp(max * b);
	return std::log(value / a) / b;
}

void AudioAnalyzerAudioProcessorEditor::timerCallback()
{
	//    if ( aP.checkForNewAnalyserData() ) {
	repaint(plotFrame);
	//    }
}

void AudioAnalyzerAudioProcessorEditor::mouseMove(const juce::MouseEvent& e)
{
	int lW = 60;
	int lH = 30;
	int x = e.x;
	if (e.eventComponent != this) {
		e.getEventRelativeTo(this); // no Work!!
	}

	if (e.eventComponent == this && e.y > lH) {

		float width = cS->newW;
		//    float height = cS->newH;
		float minFreq = cS->minFreq;
		float maxFreq = cS->maxFreq;
		//    float sampleRate = maxFreq * 2;

		float freq = juce::jmap((float)x, 0.0f, width, minFreq, maxFreq);

		if (!cS->setLiner) {
			freq = logScale(freq, minFreq, maxFreq);
		}

		juce::String freqText = (freq < 1000) ? juce::String(round(freq)) : juce::String(round((freq / 1000) * 10) / 10) + "k";


		float xPos = (x > (width - (lW / 2))) ? (width - lW) : e.x - (lW / 2);
		xPos = (x < (lW / 2)) ? 0 : xPos;

		freqLabel.setBounds(int(xPos), 0, lW, lH);
		freqLabel.setJustificationType(juce::Justification::centred);
		freqLabel.setText(freqText, juce::dontSendNotification);
	}
}

void AudioAnalyzerAudioProcessorEditor::mouseEnter(const juce::MouseEvent& e)
{
	panel.setVisible(true);
	if (cS->menuBarHide && cS->mode <= 2) {
		freqLabel.setVisible(true);
	}
}

void AudioAnalyzerAudioProcessorEditor::mouseExit(const juce::MouseEvent& e)
{
	panel.setVisible(false);
	freqLabel.setVisible(false);
}
