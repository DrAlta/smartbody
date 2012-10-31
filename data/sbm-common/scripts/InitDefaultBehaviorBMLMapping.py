UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'social_smile',\
('<face amount="0.7" au="12" end="1.8" side="BOTH" start="0.0" ready="0.5" relax="1.2" type="facs"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'duchenne_smile',\
('<face amount="0.7" au="6" end="2.2" side="BOTH" start="0.0" ready="0.4" relax="1.3" type="facs"/>',\
 '<face amount="0.45" au="99" end="2.0" side="BOTH" start="0.0" ready="0.3" relax="1.3" type="facs"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'big_smile',\
('<face amount="0.45" au="99" end="2.2" side="BOTH" start="0.0" ready="0.3" relax="1.3" type="facs"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'small_smile',\
('<face amount="0.6" au="100" end="1.8" side="BOTH" start="0.0"  ready="0.5" relax="1.2" type="facs"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'blink',\
('<face id="blink" type="facs" au="45" amount="0.8" start="0" relax="0.2" end="0.45" side="BOTH"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'shortblink',\
('<face id="blink" type="facs" au="45" amount="0.8" start="0" relax="0.1" end="0." side="BOTH"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'concern_mild',\
('<face amount="0.35" au="103" end="1.9" relax=".9" side="BOTH" start="0" stroke=".3" type="facs"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'concern',\
('<face amount="0.55" au="103" end="2.1" relax=".9" side="BOTH" start="0" stroke=".4" type="facs"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'lip_press',\
('<face id="surprise_f1" type="facs" au="24" amount="0.85" start="0" ready="0.43" relax="1.0" end="1.4" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'lip_corner_depressor',\
('<face type="facs" au="15" amount="0.4" start="0" ready="0.43" relax="1.0" end="1.4" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'tilt_rt',\
('<head type="TOSS" amount="-0.25" repeats="0.5" start="0" ready="0.5" relax="0.7" end="1.4"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'tilt_lf',\
('<head type="TOSS" amount="0.25" repeats="0.5" start="0" ready="0.5" relax="0.7" end="1.4"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'wiggle',\
('<head type="WIGGLE" amount="0.3" start="$ready_time"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'half_nod',\
('<head id="action" type="nod" velocity="1" amount="-0.05" repeats="0.5" sbm:smooth="0.35" start="0"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'half_nod_up',\
('<head id="action" type="nod" velocity="1" amount="0.05" repeats="0.5" sbm:smooth="0.35" start="0"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'nod_toss',\
('<head type="TOSS" amount="0.25" repeats="0.5" start="0" end="3.0"/>',\
 '<head id="anticipation" type="nod" velocity="1" amount="-0.02" repeats="0.5" start="0"/>',\
 '<head id="action" type="nod" velocity="0.7" amount="0.25" repeats="1" start="anticipation:relax" relax="anticipation:relax+1.4" />',\
 '<head id="overshoot" type="nod" velocity="0.5" amount="0.05" repeats="0.5" start="action:relax"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'nod_agree_lf',\
('<head type="TOSS" amount="0.25" repeats="0.5" start="0" end="3.0"/>',\
 '<head id="anticipation" type="nod" velocity="1" amount="-0.02" repeats="0.5" start="0"/>',\
 '<head id="action" type="nod" velocity="0.7" amount="0.25" repeats="1" start="anticipation:relax" relax="anticipation:relax+1.4" />',\
 '<head id="overshoot" type="nod" velocity="0.5" amount="0.05" repeats="0.5" start="action:relax"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'nod_agree_rt',\
('<head type="TOSS" amount="-0.25" repeats="0.5" start="0" end="3.0"/>',\
 '<head id="anticipation" type="nod" velocity="1" amount="-0.02" repeats="0.5" start="0"/>',\
 '<head id="action" type="nod" velocity="0.7" amount="0.25" repeats="1" start="anticipation:relax" relax="anticipation:relax+1.4" />',\
 '<head id="overshoot" type="nod" velocity="0.5" amount="0.05" repeats="0.5" start="action:relax"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'toss_nod',\
('<head amount="0.15" priority="5" start="0.2" relax="0.7" repeats="0.5" type="NOD"/>',\
 '<head type="TOSS" amount="0.20" repeats="0.5" start="0" ready="0.2" end="0.7"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'tilt_rt_nod',\
('<head id="action" type="nod" amount="0.10" repeats="0.5" start="0"/>',\
 '<head type="TOSS" amount="-0.2" repeats="0.5" start="0" ready="0.4" relax="0.8" end="1.2"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'tilt_lf_nod',\
('<head id="action" type="nod" amount="0.10" repeats="0.5" start="0"/>',\
 '<head type="TOSS" amount="0.2" repeats="0.5" start="0" ready="0.4" relax="0.8" end="1.2"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'headtiltleft',\
('<head sbm:state-name="paramHeadTilt" stroke="0" relax="$end_time" type="PARAMETERIZED" x="-30" y="0" />',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'headtiltright',\
('<head sbm:state-name="paramHeadTilt" stroke="0" relax="$end_time" type="PARAMETERIZED" x="-30" y="0" />',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'headtiltleft_nod',\
('<head id="action" type="nod" velocity="1" amount="0.05" repeats="0.5" start="0"/>',\
 '<head sbm:state-name="paramHeadTilt" stroke="0" relax="$end_time" type="PARAMETERIZED" x="-20" y="5" />',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'headtiltright_nod',\
('<head id="action" type="nod" velocity="1" amount="0.05" repeats="0.5" start="0"/>',\
 '<head sbm:state-name="paramHeadTilt" stroke="0" relax="$end_time" type="PARAMETERIZED" x="20" y="5" />',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'sweep',\
('<head type="TOSS" amount="-0.10" repeats="0.5" start="0" end="1.0"/>',\
 '<head type="nod" amount="0.15" repeats="0.5" start="0" end="0.5"/>',\
 '<head type="shake" amount="-0.25" repeats="0.5" start="0" end="1.0"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'shake_nod',\
('<head id="action2" type="shake" amount="-0.2" repeats="0.5" start="0" relax="0.8"/>',\
 '<head id="action4" type="nod" velocity="1" amount="0.15" repeats="1" start="action2:start" relax="action2:relax+0.5"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'shake_nod_small',\
('<head id="action2" type="shake" amount="-0.1" repeats="0.5" start="0" relax="0.8"/>',\
 '<head id="action4" type="nod" velocity="1" amount="0.08" repeats="1" start="action2:start" relax="action2:relax+0.5"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'big_nod',\
('<face id="blink" type="facs" au="45" amount="0.4" start="0" end="0.1" side="BOTH"/>',\
 '<head id="anticipation" type="nod" velocity="1" amount="-0.02" repeats="0.5" start="blink:start"/>',\
 '<head id="action" type="nod" velocity="0.8" amount="0.1" repeats="1" start="anticipation:relax" relax="anticipation:relax+0.8" />',\
 '<head id="overshoot" type="nod" velocity="0.8" amount="0.05" repeats="0.5" start="action:relax"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'nod',\
('<face id="blink" type="facs" au="45" amount="0.4" start="0" end="0.1" side="BOTH"/>',\
 '<head id="anticipation" type="nod" velocity="1" amount="-0.01" repeats="0.5" start="blink:start"/>',\
 '<head id="action" type="nod" velocity="1" amount="0.1" repeats="1" start="anticipation:relax" relax="anticipation:relax+0.5" />',\
 '<head id="overshoot" type="nod" velocity="0.5" amount="0.04" repeats="0.5" start="action:relax"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'small_nod',\
('<face id="blink" type="facs" au="45" amount="0.4" start="0" end="0.2" side="BOTH"/>',\
 '<head id="action" type="nod" velocity="1" amount="0.05" repeats="1" start="blink:start"/>',\
 '<head id="overshoot" type="nod" velocity="1" amount="0.01" repeats="0.5" start="action:relax"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'big_shake',\
('<head id="action" type="shake" amount="0.15" velocity="0.9" repeats="1" start="anticipation:relax"/>',\
 '<head id="overshoot" type="shake" amount="0.03" repeats="0.5" start="action:relax" />',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'shake',\
('<head id="action" type="shake" amount="0.1" repeats="1" start="anticipation:relax"/>',\
 '<head id="overshoot" type="shake" amount="0.03" repeats="0.5" start="action:relax" />',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'small_shake',\
('<head id="action" type="shake" amount="0.03" repeats="1" start="anticipation:relax"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'shake_twice',\
('<head id="action" type="shake" amount="0.1" repeats="2" velocity="1.2" start="anticipation:relax"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'big_shake_closed_eyes',\
('<face id="heavy_eyelid" type="facs" au="45" amount="0.7" start="0" ready="0.5" relax="0.8" end="1"/>',\
 '<head id="action" type="shake" amount="0.15" velocity="0.9" repeats="1" start="anticipation:relax"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'shake_closed_eyes',\
('<face id="heavy_eyelid" type="facs" au="45" amount="0.7" start="0" ready="0.5" relax="0.8" end="1"/>',\
 '<head id="action" type="shake" amount="0.1" repeats="1" start="anticipation:relax"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'small_shake_closed_eyes',\
('<face id="heavy_eyelid" type="facs" au="45" amount="0.7" start="0" ready="0.2" relax="0.4" end="0.6"/>',\
 '<head id="action" type="shake" amount="0.03" repeats="1" start="anticipation:relax"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'shake_twice_closed_eyes',\
('<face id="heavy_eyelid" type="facs" au="45" amount="0.7" start="0" ready="0.5" relax="1.2" end="1.4"/>',\
 '<head id="action" type="shake" amount="0.1" repeats="2" velocity="1.2" start="anticipation:relax"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_you',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="YOU"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_me',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="ME"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_negation',\
('<gesture stroke="0" priority="3" mode="BOTH_HANDS" lexeme="NEGATION"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_contrast',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="CONTRAST"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_assumption',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="ASSUMPTION"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_rhetorical',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="RHETORICAL"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_inclusivity',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="INCLUSIVITY"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_question',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="QUESTION"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_obligation',\
('<gesture ready="$start_time" stroke="0" priority="3" mode="RIGHT_HAND" lexeme="OBLIGATION"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_greeting',\
('<gesture ready="$start_time" stroke="0" priority="3" mode="RIGHT_HAND" lexeme="GREETING"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'brow_frown',\
('<face type="facs" au="4" amount="0.5" relax="$ready_time" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'brow_raise',\
('<face type="facs" au="2" amount="0.5" relax="$ready_time" priority="3"/>',\
 '<face type="facs" au="1" amount="0.5" relax="$ready_time" priority="3"/>'))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'brow_raise_small',\
('<face type="facs" au="2" amount="0.3" relax="$ready_time" priority="3"/>',\
 '<face type="facs" au="1" amount="0.3" relax="$ready_time" priority="3"/>'))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'smile',\
('<face amount="0.7" au="12" end="$relax_time" side="BOTH" start="0.0" type="facs"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_right',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="RIGHT""/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_left',\
('<gesture ready="$start_time" stroke="0" priority="3" mode="LEFT_HAND" lexeme="LEFT"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_chop',\
('<gesture ready="$start_time" stroke="0" priority="3" mode="RIGHT_HAND" lexeme="CHOP"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_empty',\
('<gesture ready="$start_time" stroke="0" priority="3" mode="RIGHT_HAND" lexeme="EMPTY"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_rubneck',\
('<gesture strok="0" priority="3" mode="RIGHT_HAND" lexeme="`RUBNECK"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_rubneckloop',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="RUBNECKLOOP"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_rubhead',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="RUBHEAD"/>"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_rubheadloop',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="RUBHEADLOOP" />"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_contemplate',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="CONTEMPLATE" />"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_contemplateloop',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="CONTEMPLATELOOP" />"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_dismissrarm',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="DISMISSRARM" />"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_grabchinloop',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="GRABCHINLOOP" />"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_horizontal',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="HORIZONTAL" />"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_aversion_upleft',\
('<gaze sbm:joint-range="EYES" target="$target" direction="UPLEFT" angle="15"/>',\
 '<gaze sbm:joint-range="EYES" direction="DOWNLEFT" angle="0" start="0.8" target="$target"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_aversion_upright',\
('<gaze sbm:joint-range="EYES" target="$target" direction="UPRIGHT" angle="15"/>',\
 '<gaze sbm:joint-range="EYES" direction="DOWNLEFT" angle="0" start="0.8" target="$target"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_aversion_downleft',\
('<gaze sbm:joint-range="EYES" target="$target" direction="DOWNLEFT" angle="15"/>',\
 '<gaze sbm:joint-range="EYES" direction="DOWNLEFT" angle="0" start="0.8" target="$target"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_aversion_downright',\
('<gaze sbm:joint-range="EYES" target="$target" direction="DOWNRIGHT" angle="15"/>',\
 '<gaze sbm:joint-range="EYES" direction="DOWNLEFT" angle="0" start="0.8" target="$target"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_aversion_left',\
('<gaze sbm:joint-range="EYES" target="$target" direction="LEFT" angle="15"/>',\
 '<gaze sbm:joint-range="EYES" direction="DOWNLEFT" angle="0" start="0.8" target="$target"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_aversion_right',\
('<gaze sbm:joint-range="EYES" target="$target" direction="RIGHT" angle="15"/>',\
 '<gaze sbm:joint-range="EYES" direction="DOWNLEFT" angle="0" start="0.8" target="$target"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_focus',\
('<gaze id="gaze_focus" sbm:fade-in="0.2"/>',\
 '<gaze id="gaze_focus" target="$target" priority="3"/>',\
 '<gaze id="gaze_focus" target="$target" sbm:fade-out="0.2" start="1.5"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_weak_focus',\
('<gaze id="gaze_weak_focus" sbm:fade-in="0.2"/>',\
 '<gaze id="gaze_weak_focus" target="$target" priority="3"/>',\
 '<gaze id="gaze_weak_focus" target="$target" sbm:fade-out="0.2" start="0.8"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_cursory',\
('<gaze target="$target" sbm:joint-range="NECK EYES" priority="3"/>',\
 '<gaze target="$prev_target" sbm:joint-range="NECK EYES" start="1.5" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_cursory_2',\
('<gaze target="$target" direction="POLAR" angle="0" sbm:joint-range="HEAD EYES" start="0.0" priority="3"/>',\
 '<gaze target="$prev_target" direction="POLAR" angle="0" sbm:joint-range="HEAD EYES" start="1.5" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_quick_avert',\
('<gaze target="alesia" id="avert" direction="DOWNLEFT" angle="12" sbm:joint-range="EYES" sbm:joint-speed="100" start="0" end="0.4"/>',\
 '<gaze target="alesia" id="avert-ret" sbm:joint-range="EYES" sbm:joint-speed="100" start="0.6"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_saccade',\
('<gaze target="$target" direction="RIGHT" angle="5" sbm:joint-range="EYES" start="0" priority="3"/>',\
 '<gaze target="$target" direction="LEFT" angle="5" sbm:joint-range="EYES" start="0.5" priority="3"/>',\
 '<gaze target="$target" direction="POLAR" angle="0" sbm:joint-range="EYES" start="1.0" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'head_to_right',\
('<gaze sbm:handle="head_to_right" target="$target" sbm:fade-in="0.2"/>',\
 '<gaze sbm:handle="head_to_right" sbm:joint-range="EYES NECK" target="$target" direction="RIGHT" angle="15"/>',\
 '<gaze sbm:handle="head_to_right" target="$target" sbm:fade-out="0.2" start="1.0"/>'))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'head_to_left',\
('<gaze sbm:handle="head_to_left" target="$target" sbm:fade-in="0.2"/>',\
 '<gaze sbm:handle="head_to_left" sbm:joint-range="EYES NECK" target="$target" direction="LEFT" angle="15"/>',\
 '<gaze sbm:handle="head_to_left" target="$target" sbm:fade-out="0.2" start="1.0"/>'))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'head_up',\
('<gaze sbm:handle="head_up" target="$target" sbm:fade-in="0.2"/>',\
 '<gaze sbm:handle="head_up" sbm:joint-range="EYES NECK" target="$target" direction="UP" angle="15"/>',\
 '<gaze sbm:handle="head_up" target="$target" sbm:fade-out="0.2" start="1.0"/>'))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'head_down',\
('<gaze sbm:handle="head_down" target="$target" sbm:fade-in="0.2"/>',\
 '<gaze sbm:handle="head_down" sbm:joint-range="EYES NECK" target="$target" direction="DOWN" angle="15"/>',\
 '<gaze sbm:handle="head_down" target="$target" sbm:fade-out="0.2" start="1.0"/>'))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'emo-surprise2',\
('<face id="surprise_f1" type="facs" au="1" amount="0.2" sbm:rampup="0.33" sbm:rampdown="0.4" start="0" end="2.0" priority="3"/>',\
 '<face id="surprise_f2" type="facs" au="2" amount="0.5" sbm:rampup="0.33" sbm:rampdown="0.4" start="surprise_f1:start" relax="surprise_f1:relax" end="surprise_f1:end" priority="3"/>',\
 '<face id="surprise_f3" type="facs" au="5" amount="0.25" sbm:rampup="0.25" sbm:rampdown="0.25" start="surprise_f1:start+0.03" relax="surprise_f1:relax-0.01" end="surprise_f1:end" priority="3"/>',\
 '<face id="surprise_f4" type="facs" au="25" amount="0.1" sbm:rampup="0.3" sbm:rampdown="0.5" start="surprise_f1:start+0.2" relax="surprise_f1:relax" end="surprise_f1:end-0.05" priority="3"/>',\
 '<face id="surprise_f5" type="facs" au="27" amount="0.05" sbm:rampup="0.3" sbm:rampdown="0.5" start="surprise_f1:start+0.2" relax="surprise_f1:relax" end="surprise_f1:end-0.1" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'emo-joy',\
('<face id="joy_f1" type="facs" au="12" amount="1" sbm:rampup="0.5" sbm:rampdown="0.8" start="0" ready="0.5" relax="2.5" end="3.5" priority="3"/>',\
 '<face id="joy_f2" type="facs" au="6" amount="0.2" sbm:rampup="0.2" sbm:rampdown="2.03" start="joy_f1:start-0.05" ready="joy_f1:ready" relax="joy_f1:relax" end="joy_f1:end+0.03" priority="3"/>',\
 '<face id="joy_f3" type="facs" au="27" amount="0.1" sbm:rampup="0.33" sbm:rampdown="0.45" start="joy_f1:start+0.12" relax="joy_f1:relax+0.15" priority="3"/>',\
 '<face id="joy_f4" type="facs" au="1" amount="0.15" sbm:rampup="0.33" sbm:rampdown="0.4" sbm:duration="1.0" start="joy_f1:start+0.1" relax="joy_f1:relax+0.15" priority="3"/>',\
 '<face id="joy_f5" type="facs" au="2" amount="0.6" sbm:rampup="0.33" sbm:rampdown="0.4" sbm:duration="1.0" start="joy_f4:start" relax="joy_f4:relax" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'emo-surprise',\
('<face id="surprise_f1" type="facs" au="1" amount="0.1" sbm:rampup="0.33" sbm:rampdown="0.4" start="0" ready="0.33" relax="1.0" priority="3"/>',\
 '<face id="surprise_f2" type="facs" au="2" amount="0.5" sbm:rampup="0.33" sbm:rampdown="0.4" start="surprise_f1:start" relax="surprise_f1:relax" end="surprise_f1:end" priority="3"/>',\
 '<face id="surprise_f3" type="facs" au="5" amount="0.25" sbm:rampup="0.25" sbm:rampdown="0.25" start="surprise_f1:start+0.03" relax="surprise_f1:relax-0.01" end="surprise_f1:end" priority="3"/>',\
 '<face id="surprise_f4" type="facs" au="25" amount="0.1" sbm:rampup="0.3" sbm:rampdown="0.5" start="surprise_f1:start+0.2" relax="surprise_f1:relax" end="surprise_f1:end-0.05" priority="3"/>',\
 '<face id="surprise_f5" type="facs" au="27" amount="0.05" sbm:rampup="0.3" sbm:rampdown="0.5" start="surprise_f1:start+0.2" relax="surprise_f1:relax" end="surprise_f1:end-0.1" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'emo-sadness',\
('<face id="sad_f1" type="facs" au="1" amount="0.7" sbm:rampup="0.33" sbm:rampdown="0.5" start="0" ready="1.03" relax="3.5" priority="3"/>',\
 '<face id="sad_f2" type="facs" au="4" amount="0.2" sbm:rampup="0.4" sbm:rampdown="0.5" start="sad_f1:start" relax="sad_f1:relax" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'attitude-agree',\
('<head id="agree_h1" type="NOD" amount="0.2" repeats="1.5" velocity="3" sbm:smooth="0.7" start="0.0" ready="0.25" relax="0.8" end="1.5" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'attitude-disagree',\
('<gaze id="disagree_g1" target="$target" start="0.0" end="0.5"/>',\
 '<head id="disagree_h1" type="SHAKE" repeats="1" amount="0.15" velocity="1" sbm:smooth="0.4" start="disagree_g1:end"/>',\
 '<head id="disagree_h2" type="SHAKE" repeats="1" amount="0.1" velocity="1.5" sbm:smooth="0.7" start="disagree_h1:end-0.1"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'attitude-like',\
('<face id="like_f1" type="facs" au="12" amount="1" sbm:rampup="0.6" sbm:rampdown="1" start="0" ready="0.8" relax="2.0" end="3.0" priority="3"/>',\
 '<face id="like_f2" type="facs" au="6" amount="0.2" sbm:rampup="0.2" sbm:rampdown="1.03" start="like_f1:start-0.05" ready="like_f1:ready" relax="like_f1:relax" end="like_f1:end+0.03" priority="3"/>',))

