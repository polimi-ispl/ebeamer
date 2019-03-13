#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
//==============================================================================

TileComponent::TileComponent()
{
}

TileComponent::~TileComponent()
{
}

//==============================================================================

void TileComponent::paint(Graphics& g)
{
    Path path;

    path.startNewSubPath(corners[0][0]);
    path.lineTo(corners[1][0]);
    path.lineTo(corners[1][1]);
    path.lineTo(corners[0][1]);
    path.closeSubPath();

    g.setColour(tileColour);
    g.fillPath(path);

    g.setColour(Colours::black);
    PathStrokeType strokeType(0.5f);
    g.strokePath(path, strokeType);
}

void TileComponent::resized()
{
}

//==============================================================================
//==============================================================================

GridComponent::GridComponent()
{
    for(int i = 0; i < TILE_ROW_COUNT; i++){
        for(int j = 0; j < TILE_COL_COUNT; j++){
            addAndMakeVisible (tiles[i][j]);
        }
    }

    #ifdef PLANAR_MODE

    // 2D grid

    float w = SCENE_WIDTH;
    float h = w/2;

    float angle_diff = PI / TILE_COL_COUNT;
    float radius_diff = h / TILE_ROW_COUNT;

    for(int i = 0; i <= TILE_ROW_COUNT; i++){
        // Linear
        // float radius = h - i * radius_diff;

        // Square
        // float radius = h - h * pow( (float)i / TILE_ROW_COUNT, 2 );

        // Inverse square
        // float radius = h * sqrt( (float)(TILE_ROW_COUNT - i) / TILE_ROW_COUNT );

        // Exponential
        float radius = h - h * ( exp( (float)i / TILE_ROW_COUNT ) - 1 ) / ( exp( 1 ) - 1 );

        for(int j = 0; j <= TILE_COL_COUNT; j++){
            float angle = j*angle_diff;

            vertices[i][j].setX( w/2 - radius*cos(angle));
            vertices[i][j].setY( h  - radius*sin(angle));
        }
    }

    #else

    // 3D grid

    float w = SCENE_WIDTH;
    float h = w;
    float radius = w/2;
    float pov = radius + radius * PERSPECTIVE_RATIO;

    float R_angle_diff = PI / TILE_ROW_COUNT;
    float C_angle_diff = PI / TILE_COL_COUNT;

    for(int i = 0; i <= TILE_ROW_COUNT; i++){
        float R_angle = i * R_angle_diff;

        for(int j = 0; j <= TILE_COL_COUNT; j++){
            float C_angle = PI - (j * C_angle_diff);

            vertices[i][j].setX(
                                radius +
                                    ( pov * ( radius * cos(C_angle) * sin(R_angle) ) )
                                    /
                                    ( pov + ( radius * sin(C_angle) * sin(R_angle) ) )
                                );

            vertices[i][j].setY(
                                radius -
                                    ( pov * ( radius * cos(R_angle) ) )
                                    /
                                    ( pov + ( radius * sin(C_angle) * sin(R_angle) ) )
                                );
        }
    }

    #endif
}

GridComponent::~GridComponent()
{
}

//==============================================================================

void GridComponent::resized()
{
    for(int i = 0; i < TILE_ROW_COUNT; i++){
        for(int j = 0; j < TILE_COL_COUNT; j++){

            tiles[i][j].corners[0][0] = vertices[i  ][j  ];
            tiles[i][j].corners[1][0] = vertices[i+1][j  ];
            tiles[i][j].corners[0][1] = vertices[i  ][j+1];
            tiles[i][j].corners[1][1] = vertices[i+1][j+1];

            tiles[i][j].setBounds(getLocalBounds());

            #ifdef PLANAR_MODE

            // 2D grid

            if(i < TILE_ROW_COUNT/4)
                tiles[i][j].tileColour = Colours::red.darker(0.9);

            if(TILE_ROW_COUNT/4 <= i && i < TILE_ROW_COUNT/2)
                tiles[i][j].tileColour = Colours::yellow.darker(0.9);

            if(i >= TILE_ROW_COUNT/2)
                tiles[i][j].tileColour = Colours::green.darker(0.9);

            #else

            // 3D grid

            if( (i + j)% 2 == 0 ){
                if( i == floor(TILE_ROW_COUNT/2) )
                    tiles[i][j].tileColour = Colours::white;
                else
                    tiles[i][j].tileColour = Colours::grey;
            }

            #endif
        }
    }
}

void GridComponent::updateEnergy(float* energy)
{
    for(int j = 0; j < TILE_COL_COUNT; j++){
        int level = TILE_ROW_COUNT - ceil(TILE_ROW_COUNT * energy[j]);

        for(int i = 0; i < TILE_ROW_COUNT; i++){

            #ifdef PLANAR_MODE

            // 2D grid

            if(i < level){
                if(i < TILE_ROW_COUNT/4)
                    tiles[i][j].tileColour = Colours::red.darker(0.9);

                if(TILE_ROW_COUNT/4 <= i && i < TILE_ROW_COUNT/2)
                    tiles[i][j].tileColour = Colours::yellow.darker(0.9);

                if(i >= TILE_ROW_COUNT/2)
                    tiles[i][j].tileColour = Colours::green.darker(0.9);
            } else {
                if(i < TILE_ROW_COUNT/4)
                    tiles[i][j].tileColour = Colours::red;

                if(TILE_ROW_COUNT/4 <= i && i < TILE_ROW_COUNT/2)
                    tiles[i][j].tileColour = Colours::yellow;

                if(i >= TILE_ROW_COUNT/2)
                    tiles[i][j].tileColour = Colours::green;
            }

            #else

            // 3D grid

            #endif
        }
    }

    repaint();
}

//==============================================================================
//==============================================================================

BeamComponent::BeamComponent()
{
    position = 0;
}

BeamComponent::~BeamComponent()
{
}

//==============================================================================

void BeamComponent::paint(Graphics& g)
{
    #ifdef PLANAR_MODE

    Path path;

    path.startNewSubPath(0, 0);
    path.cubicTo( width, -SCENE_WIDTH/3,  width, -SCENE_WIDTH/2, 0, -SCENE_WIDTH/2);
    path.cubicTo(-width, -SCENE_WIDTH/2, -width, -SCENE_WIDTH/3, 0, 0);
    path.closeSubPath();

    path.applyTransform(AffineTransform::rotation( (PI/2) * position));
    path.applyTransform(AffineTransform::translation(SCENE_WIDTH/2, SCENE_WIDTH/2));

    g.setColour(Colours::lightblue);
    g.fillPath(path);

    g.setColour (Colours::blue);
    PathStrokeType strokeType(2);
    g.strokePath(path, strokeType);

    #else

    #endif
}

void BeamComponent::resized()
{
}

void BeamComponent::move(float new_position)
{
    position = new_position;
    repaint();
}

void BeamComponent::scale(float new_width)
{
    width = new_width * SCENE_WIDTH/10;
    repaint();
}

//==============================================================================
//==============================================================================

SceneComponent::SceneComponent()
{
    addAndMakeVisible (grid);
    for(int i = 0; i < NUM_BEAMS; i++)
      addAndMakeVisible (beams[i]);

    beams[0].move(-0.5);
    beams[0].scale(0.25);

    beams[1].move(0.5);
    beams[1].scale(0.5);
}

SceneComponent::~SceneComponent()
{
}

//==============================================================================

void SceneComponent::paint(Graphics& g)
{
    g.fillAll(Colours::black);
}

void SceneComponent::resized()
{
    grid.setBounds(getLocalBounds());
    for(int i = 0; i < NUM_BEAMS; i++)
      beams[i].setBounds(getLocalBounds());
}

void SceneComponent::updateEnergy(float* energy)
{
    grid.updateEnergy(energy);
}

//==============================================================================
//==============================================================================

JucebeamAudioProcessorEditor::JucebeamAudioProcessorEditor (JucebeamAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (GUI_WIDTH, GUI_HEIGHT);

    // processor.addChangeListener (this);
    addAndMakeVisible (scene);

    NormalisableRange<float> parameterRange;
    // Steering direction slider
    parameterRange = processor.steeringBeam[0]->getNormalisableRange();
    steeringBeam1Slider.setRange(parameterRange.start,parameterRange.end,0.01);
    steeringBeam1Slider.setValue(processor.steeringBeam[0]->get());
    steeringBeam1Slider.addListener(this);
    steeringBeam1Slider.setSliderStyle(Slider::LinearHorizontal);
    steeringBeam1Slider.setTextBoxStyle(Slider::TextBoxRight,false,60,20);
    steeringBeam1Slider.setValue(processor.steeringBeam[0]->get());
    addAndMakeVisible(steeringBeam1Slider);
    
    parameterRange = processor.steeringBeam[1]->getNormalisableRange();
    steeringBeam2Slider.setRange(parameterRange.start,parameterRange.end,0.01);
    steeringBeam2Slider.setValue(processor.steeringBeam[1]->get());
    steeringBeam2Slider.addListener(this);
    steeringBeam2Slider.setSliderStyle(Slider::LinearHorizontal);
    steeringBeam2Slider.setTextBoxStyle(Slider::TextBoxRight,false,60,20);
    steeringBeam2Slider.setValue(processor.steeringBeam[1]->get());
    addAndMakeVisible(steeringBeam2Slider);

    steerLabel.setText("STEER", NotificationType::dontSendNotification);
    steerLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(steerLabel);

    widthLabel.setText("WIDTH", NotificationType::dontSendNotification);
    widthLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(widthLabel);
    
    parameterRange = processor.widthBeam[0]->getNormalisableRange();
    widthBeam1Knob.setRange(parameterRange.start,parameterRange.end,0.01);
    widthBeam1Knob.setValue(processor.widthBeam[0]->get());
    widthBeam1Knob.addListener(this);
    widthBeam1Knob.setSliderStyle(Slider::Rotary);
    widthBeam1Knob.setTextBoxStyle(Slider::TextBoxRight,false,LABEL_WIDTH,LABEL_HEIGHT);
    widthBeam1Knob.setValue(processor.widthBeam[0]->get());
    addAndMakeVisible(widthBeam1Knob);
    
    parameterRange = processor.widthBeam[1]->getNormalisableRange();
    widthBeam2Knob.setRange(parameterRange.start,parameterRange.end,0.01);
    widthBeam2Knob.setValue(processor.widthBeam[1]->get());
    widthBeam2Knob.addListener(this);
    widthBeam2Knob.setSliderStyle(Slider::Rotary);
    widthBeam2Knob.setTextBoxStyle(Slider::TextBoxLeft,false,LABEL_WIDTH,LABEL_HEIGHT);
    widthBeam2Knob.setValue(processor.widthBeam[1]->get());
    addAndMakeVisible(widthBeam2Knob);

    panLabel.setText("PAN", NotificationType::dontSendNotification);
    addAndMakeVisible(panLabel);
    
    parameterRange = processor.panBeam[0]->getNormalisableRange();
    panBeam1Knob.setRange(parameterRange.start,parameterRange.end,0.01);
    panBeam1Knob.setValue(processor.panBeam[0]->get());
    panBeam1Knob.addListener(this);
    panBeam1Knob.setSliderStyle(Slider::Rotary);
    panBeam1Knob.setTextBoxStyle(Slider::TextBoxRight,false,LABEL_WIDTH,LABEL_HEIGHT);
    panBeam1Knob.setValue(processor.panBeam[0]->get());
    addAndMakeVisible(panBeam1Knob);
    
    parameterRange = processor.panBeam[0]->getNormalisableRange();
    panBeam2Knob.setRange(parameterRange.start,parameterRange.end,0.01);
    panBeam2Knob.setValue(processor.panBeam[1]->get());
    panBeam2Knob.addListener(this);
    panBeam2Knob.setSliderStyle(Slider::Rotary);
    panBeam2Knob.setTextBoxStyle(Slider::TextBoxLeft,false,LABEL_WIDTH,LABEL_HEIGHT);
    panBeam2Knob.setValue(processor.panBeam[1]->get());
    addAndMakeVisible(panBeam2Knob);

    gainLabel.setText("GAIN", NotificationType::dontSendNotification);
    addAndMakeVisible(gainLabel);
    
    parameterRange = processor.gainBeam[0]->getNormalisableRange();
    gainBeam1Knob.setRange(parameterRange.start,parameterRange.end,1);
    gainBeam1Knob.setValue(processor.gainBeam[0]->get());
    gainBeam1Knob.addListener(this);
    gainBeam1Knob.setSliderStyle(Slider::Rotary);
    gainBeam1Knob.setTextBoxStyle(Slider::TextBoxRight,false,LABEL_WIDTH,LABEL_HEIGHT);
    gainBeam1Knob.setValue(processor.gainBeam[0]->get());
    addAndMakeVisible(gainBeam1Knob);
    
    parameterRange = processor.gainBeam[1]->getNormalisableRange();
    gainBeam2Knob.setRange(parameterRange.start,parameterRange.end,1);
    gainBeam2Knob.setValue(processor.gainBeam[1]->get());
    gainBeam2Knob.addListener(this);
    gainBeam2Knob.setSliderStyle(Slider::Rotary);
    gainBeam2Knob.setTextBoxStyle(Slider::TextBoxLeft,false,LABEL_WIDTH,LABEL_HEIGHT);
    gainBeam2Knob.setValue(processor.gainBeam[1]->get());
    addAndMakeVisible(gainBeam2Knob);

    muteLabel.setText("MUTE", NotificationType::dontSendNotification);
    addAndMakeVisible(muteLabel);
    
    beam1MuteButton.addListener(this);
    beam1MuteButton.setButtonText("1");
    addAndMakeVisible(beam1MuteButton);
    
    beam2MuteButton.addListener(this);
    beam2MuteButton.setButtonText("2");
    addAndMakeVisible(beam2MuteButton);

}

JucebeamAudioProcessorEditor::~JucebeamAudioProcessorEditor()
{
}

//==============================================================================

void JucebeamAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (15.0f);

}

void JucebeamAudioProcessorEditor::resized()
{
    #ifdef PLANAR_MODE
    
      scene.setBounds( (GUI_WIDTH - SCENE_WIDTH)/2, (GUI_HEIGHT/3 - SCENE_HEIGHT)/2, SCENE_WIDTH, SCENE_HEIGHT);
    
      steeringBeam1Slider.setBounds( GUI_WIDTH/8, SCENE_HEIGHT +  5, 6*GUI_WIDTH/8, 20);
      steeringBeam2Slider.setBounds( GUI_WIDTH/8, SCENE_HEIGHT + 35, 6*GUI_WIDTH/8, 20);
      steerLabel.setBounds( GUI_WIDTH/2 - 25, GUI_HEIGHT/3 + 65, 50, 20);

      widthLabel.setBounds(GUI_WIDTH/2 - 25, GUI_HEIGHT/2 - 10, 50, 20);
      widthBeam1Knob.setBounds(  GUI_WIDTH/4 - (KNOB_SIZE>>1), GUI_HEIGHT/2 - (KNOB_SIZE>>1), KNOB_SIZE+LABEL_WIDTH, KNOB_SIZE);
      widthBeam2Knob.setBounds(3*GUI_WIDTH/4 - (KNOB_SIZE>>1) - LABEL_WIDTH, GUI_HEIGHT/2 - (KNOB_SIZE>>1), KNOB_SIZE+LABEL_WIDTH, KNOB_SIZE);

      panLabel.setBounds(GUI_WIDTH/2 - 25, 5*GUI_HEIGHT/8 - 10, 50, 20);
      panBeam1Knob.setBounds(  GUI_WIDTH/4 - (KNOB_SIZE>>1), 5*GUI_HEIGHT/8 - (KNOB_SIZE>>1), KNOB_SIZE+LABEL_WIDTH, KNOB_SIZE);
      panBeam2Knob.setBounds(3*GUI_WIDTH/4 - (KNOB_SIZE>>1) - LABEL_WIDTH, 5*GUI_HEIGHT/8 - (KNOB_SIZE>>1), KNOB_SIZE+LABEL_WIDTH, KNOB_SIZE);

      gainLabel.setBounds(GUI_WIDTH/2 - 25, 6*GUI_HEIGHT/8 - 10, 50, 20);
      gainBeam1Knob.setBounds(  GUI_WIDTH/4 - (KNOB_SIZE>>1), 6*GUI_HEIGHT/8 - (KNOB_SIZE>>1), KNOB_SIZE+LABEL_WIDTH, KNOB_SIZE);
      gainBeam2Knob.setBounds(3*GUI_WIDTH/4 - (KNOB_SIZE>>1) - LABEL_WIDTH, 6*GUI_HEIGHT/8 - (KNOB_SIZE>>1), KNOB_SIZE+LABEL_WIDTH, KNOB_SIZE);

      muteLabel.setBounds(GUI_WIDTH/2 - 25, 7*GUI_HEIGHT/8 - 10, 50, 20);
      beam1MuteButton.setBounds(  GUI_WIDTH/4 - (MUTE_SIZE>>1), 7*GUI_HEIGHT/8 - (MUTE_SIZE>>1), MUTE_SIZE, MUTE_SIZE);
      beam2MuteButton.setBounds(3*GUI_WIDTH/4 - (MUTE_SIZE>>1), 7*GUI_HEIGHT/8 - (MUTE_SIZE>>1), MUTE_SIZE, MUTE_SIZE);
    
      
    #else
    
      scene.setBounds( (GUI_WIDTH/2 - SCENE_WIDTH)/2, (GUI_HEIGHT - SCENE_HEIGHT)/2, SCENE_WIDTH, SCENE_HEIGHT);
      
      steeringBeam1Slider.setBounds( 9*GUI_WIDTH/16,  5, 6*GUI_WIDTH/16, 20);
      steeringBeam2Slider.setBounds( 9*GUI_WIDTH/16, 35, 6*GUI_WIDTH/16, 20);
      steerLabel.setBounds( 3*GUI_WIDTH/4 - 25, 65, 50, 20);

      widthLabel.setBounds(3*GUI_WIDTH/4 - 25, GUI_HEIGHT/4 - 10, 50, 20);
      widthBeam1Knob.setBounds(5*GUI_WIDTH/8 - (KNOB_SIZE>>1), GUI_HEIGHT/4 - (KNOB_SIZE>>1), KNOB_SIZE, KNOB_SIZE);
      widthBeam2Knob.setBounds(7*GUI_WIDTH/8 - (KNOB_SIZE>>1), GUI_HEIGHT/4 - (KNOB_SIZE>>1), KNOB_SIZE, KNOB_SIZE);

      panLabel.setBounds(3*GUI_WIDTH/4 - 25, 7*GUI_HEIGHT/16 - 10, 50, 20);
      panBeam1Knob.setBounds(5*GUI_WIDTH/8 - (KNOB_SIZE>>1), 7*GUI_HEIGHT/16 - (KNOB_SIZE>>1), KNOB_SIZE, KNOB_SIZE);
      panBeam2Knob.setBounds(7*GUI_WIDTH/8 - (KNOB_SIZE>>1), 7*GUI_HEIGHT/16 - (KNOB_SIZE>>1), KNOB_SIZE, KNOB_SIZE);

      gainLabel.setBounds(3*GUI_WIDTH/4 - 25, 10*GUI_HEIGHT/16 - 10, 50, 20);
      gainBeam1Knob.setBounds(5*GUI_WIDTH/8 - (KNOB_SIZE>>1), 10*GUI_HEIGHT/16 - (KNOB_SIZE>>1), KNOB_SIZE, KNOB_SIZE);
      gainBeam2Knob.setBounds(7*GUI_WIDTH/8 - (KNOB_SIZE>>1), 10*GUI_HEIGHT/16 - (KNOB_SIZE>>1), KNOB_SIZE, KNOB_SIZE);

      muteLabel.setBounds(3*GUI_WIDTH/4 - 25, 13*GUI_HEIGHT/16 - 10, 50, 20);
      beam1MuteButton.setBounds(5*GUI_WIDTH/8 - (MUTE_SIZE>>1), 13*GUI_HEIGHT/16 - (MUTE_SIZE>>1), MUTE_SIZE, MUTE_SIZE);
      beam2MuteButton.setBounds(7*GUI_WIDTH/8 - (MUTE_SIZE>>1), 13*GUI_HEIGHT/16 - (MUTE_SIZE>>1), MUTE_SIZE, MUTE_SIZE);
      
    #endif

}

void JucebeamAudioProcessorEditor::buttonClicked(Button *button)
{
    if(button == &beam1MuteButton)
    {
        *(processor.muteBeam[0]) = 1 - *(processor.muteBeam[0]);
    }
    else if(button == &beam2MuteButton)
    {
        *(processor.muteBeam[1]) = 1 - *(processor.muteBeam[1]);
    }
}

void JucebeamAudioProcessorEditor::sliderValueChanged(Slider *slider)
{
    if (slider == &steeringBeam1Slider)
    {
        *(processor.steeringBeam[0]) = slider->getValue();
    }
    else if (slider == &steeringBeam2Slider)
    {
        *(processor.steeringBeam[1]) = slider->getValue();
    }
    else if (slider == &widthBeam1Knob)
    {
        *(processor.widthBeam[0]) = slider->getValue();
    }
    else if (slider == &widthBeam2Knob)
    {
        *(processor.widthBeam[1]) = slider->getValue();
    }
    else if (slider == &panBeam1Knob)
    {
        *(processor.panBeam[0]) = slider->getValue();
    }
    else if (slider == &panBeam2Knob)
    {
        *(processor.panBeam[1]) = slider->getValue();
    }
    else if (slider == &gainBeam1Knob)
    {
        *(processor.gainBeam[0]) = slider->getValue();
    }
    else if (slider == &gainBeam2Knob)
    {
        *(processor.gainBeam[1]) = slider->getValue();
    }
}

void JucebeamAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster *source)
{
    // scene.updateEnergy(processor.getDirectionalEnergy());
} 
