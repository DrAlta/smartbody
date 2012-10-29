UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'tilt_rt',\
('<head type="TOSS" amount="-0.25" repeats="0.5" start="0" ready="0.5" relax="0.7" end="1.4"/>',))
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
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'emo-surprise2',\
('<face id="surprise_f1" type="facs" au="1" amount="0.2" sbm:rampup="0.33" sbm:rampdown="0.4" start="0" end="2.0" priority="3"/>',\
 '<face id="surprise_f2" type="facs" au="2" amount="0.5" sbm:rampup="0.33" sbm:rampdown="0.4" start="surprise_f1:start" relax="surprise_f1:relax" end="surprise_f1:end" priority="3"/>',\
 '<face id="surprise_f3" type="facs" au="5" amount="0.25" sbm:rampup="0.25" sbm:rampdown="0.25" start="surprise_f1:start+0.03" relax="surprise_f1:relax-0.01" end="surprise_f1:end" priority="3"/>',\
 '<face id="surprise_f4" type="facs" au="25" amount="0.1" sbm:rampup="0.3" sbm:rampdown="0.5" start="surprise_f1:start+0.2" relax="surprise_f1:relax" end="surprise_f1:end-0.05" priority="3"/>',\
 '<face id="surprise_f5" type="facs" au="27" amount="0.05" sbm:rampup="0.3" sbm:rampdown="0.5" start="surprise_f1:start+0.2" relax="surprise_f1:relax" end="surprise_f1:end-0.1" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'wiggle',\
('<head type="WIGGLE" amount="0.2" start="$ready_time"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'waggle',\
('<head type="WAGGLE" amount="0.3" start="$ready_time"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'nod_attend',\
('<head type="NOD" amount="-0.15" repeats="1.0" velocity=".7" sbm:smooth="0.35" start="0.0" ready="0.4" relax="0.50" end="1.2" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'nod_attend_up',\
('<head type="NOD" amount=".15" repeats="1.0" velocity=".7" sbm:smooth="0.35" start="0.0" ready="0.3" relax="0.40" end="1.1" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'half_nod',\
('<head id="action" type="nod" velocity="1" amount="0.05" repeats="0.5" start="0"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'tilt_lf',\
('<head type="TOSS" amount="0.25" repeats="0.5" start="0" ready="0.5" relax="0.7" end="1.4"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'toss_lf2',\
('<head type="TOSS" amount="0.23" repeats="0.5" start="0" end="$end_time"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'sweep',\
('<head type="TOSS" amount="-0.10" repeats="0.5" start="0" end="1.0"/>',\
 '<head type="nod" amount="0.15" repeats="0.5" start="0" end="0.5"/>',\
 '<head type="shake" amount="-0.25" repeats="0.5" start="0" end="1.0"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'concern_mild',\
('<face amount="0.35" au="103" end="1.9" relax=".9" side="BOTH" start="0" stroke=".3" type="facs"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'concern',\
('<face amount="0.55" au="103" end="2.1" relax=".9" side="BOTH" start="0" stroke=".4" type="facs"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'surprise',\
('<face id="surprise_f1" type="facs" au="1" amount="0.2" start="0" ready="0.43" relax="1.0" end="1.4" priority="3"/>',\
 '<face id="surprise_f2" type="facs" au="2" amount="0.5" start="surprise_f1:start" ready="surprise_f1:ready" relax="surprise_f1:relax" end="surprise_f1:end" priority="3"/>',\
 '<face id="surprise_f3" type="facs" au="5" amount="0.25" start="surprise_f1:start+0.03" ready="surprise_f1:ready+0.03" relax="surprise_f1:relax-0.01" end="surprise_f1:end" priority="3"/>',\
 '<face id="surprise_f4" type="facs" au="25" amount="0.1" start="surprise_f1:start+0.2" ready="surprise_f1:ready+0.2" relax="surprise_f1:relax" end="surprise_f1:end-0.05" priority="3"/>',\
 '<face id="surprise_f5" type="facs" au="27" amount="0.05" start="surprise_f1:start+0.2" ready="surprise_f1:ready+0.2" relax="surprise_f1:relax" end="surprise_f1:end-0.1" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'surprise_mild',\
('<face id="surprise_f1" type="facs" au="1" amount="0.3" start="0" ready="0.43" relax="1.0" end="1.4" priority="3"/>',\
 '<face id="surprise_f2" type="facs" au="2" amount="0.4" start="surprise_f1:start" ready="surprise_f1:ready" relax="surprise_f1:relax" end="surprise_f1:end" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'lip_press',\
('<face id="surprise_f1" type="facs" au="24" amount="0.85" start="0" ready="0.43" relax="1.0" end="1.4" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'nod_toss',\
('<head type="TOSS" amount="0.25" repeats="0.5" start="0" end="3.0"/>',\
 '<head id="anticipation" type="nod" velocity="1" amount="-0.02" repeats="0.5" start="0"/>',\
 '<head id="action" type="nod" velocity="0.7" amount="0.25" repeats="1" start="anticipation:relax" relax="anticipation:relax+1.4" />',\
 '<head id="overshoot" type="nod" velocity="0.5" amount="0.05" repeats="0.5" start="action:relax"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'nod_agree',\
('<head type="TOSS" amount="0.25" repeats="0.5" start="0" end="3.0"/>',\
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
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'shake_nod',\
('<head id="anticipation2" type="shake" amount="0.06" repeats="0.5" start="0" velocity=".7"/>',\
 '<head id="action2" type="shake" amount="-0.2" repeats="0.5" start="anticipation2:start" relax="anticipation2:relax+0.5"/>',\
 '<head id="action4" type="nod" velocity="1" amount="0.15" repeats="1" start="anticipation2:start" relax="anticipation2:relax+0.5"/>',\
 '<head id="overshoot2" type="shake" amount="-0.07" repeats="0.5" start="action2:relax" />',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'shake_nod_small',\
('<head id="anticipation2" type="shake" amount="0.01" repeats="0.5" start="0"/>',\
 '<head id="action2" type="shake" amount="-0.1" repeats="1" start="0" relax="1.0"/>',\
 '<head id="action4" type="nod" amount="0.05" repeats="0.5" start="anticipation2:relax" relax="action2:relax"/>',\
 '<head id="overshoot2" type="shake" amount="-0.03" repeats="1.5" start="action2:relax" />',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'big_nod',\
('<face id="blink" type="facs" au="45" amount="0.4" start="0" end="0.1" side="BOTH"/>',\
 '<head id="anticipation" type="nod" velocity="1" amount="-0.02" repeats="0.5" start="blink:end"/>',\
 '<head id="action" type="nod" velocity="1" amount="0.2" repeats="1" start="anticipation:relax" relax="anticipation:relax+0.8" />',\
 '<head id="overshoot" type="nod" velocity="0.5" amount="0.05" repeats="0.5" start="action:relax"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'nod',\
('<face id="blink" type="facs" au="45" amount="0.4" start="0" end="0.2" side="BOTH"/>',\
 '<head id="anticipation" type="nod" velocity="1" amount="-0.01" repeats="0.5" start="blink:end"/>',\
 '<head id="action" type="nod" velocity="1" amount="0.1" repeats="1" start="anticipation:relax" relax="anticipation:relax+0.5" />',\
 '<head id="overshoot" type="nod" velocity="0.5" amount="0.04" repeats="0.5" start="action:relax"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'small_nod',\
('<face id="blink" type="facs" au="45" amount="0.4" start="0" end="0.2" side="BOTH"/>',\
 '<head id="action" type="nod" velocity="1" amount="0.05" repeats="1" start="blink:end"/>',\
 '<head id="overshoot" type="nod" velocity="1" amount="0.01" repeats="0.5" start="action:relax"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'big_shake',\
('<head id="anticipation" type="shake" amount="-0.03" repeats="0.5" start="0"/>',\
 '<head id="action" type="shake" amount="0.3" repeats="1" start="anticipation:relax"/>',\
 '<head id="overshoot" type="shake" amount="0.03" repeats="0.5" start="action:relax" />',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'shake',\
('<head id="anticipation" type="shake" amount="-0.03" repeats="0.5" start="0"/>',\
 '<head id="action" type="shake" amount="0.2" repeats="1" start="anticipation:relax"/>',\
 '<head id="overshoot" type="shake" amount="0.03" repeats="0.5" start="action:relax" />',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'small_shake',\
('<head id="anticipation" type="shake" amount="-0.03" repeats="0.5" start="0"/>',\
 '<head id="action" type="shake" amount="0.1" repeats="1" start="anticipation:relax"/>',\
 '<head id="overshoot" type="shake" amount="0.03" repeats="0.5" start="action:relax" />',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'shake_twice',\
('<head id="anticipation" type="shake" amount="-0.03" repeats="0.5" start="0"/>',\
 '<head id="action" type="shake" amount="0.1" repeats="2" velocity="1.6" start="anticipation:relax"/>',\
 '<head id="overshoot" type="shake" amount="0.03" repeats="0.5" start="action:relax" />',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'big_shake_closed_eyes',\
('<face id="heavy_eyelid" type="facs" au="5" amount="-0.5" start="0" ready="0.5" end="1"/>  ',\
 '<head id="anticipation" type="shake" amount="-0.03" repeats="0.5" start="0"/>',\
 '<head id="action" type="shake" amount="0.3" repeats="1" start="anticipation:relax"/>',\
 '<head id="overshoot" type="shake" amount="0.03" repeats="0.5" start="action:relax" />',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'shake_closed_eyes',\
('<face id="heavy_eyelid" type="facs" au="5" amount="-0.5" start="0" ready="0.5" end="1"/>  ',\
 '<head id="anticipation" type="shake" amount="-0.03" repeats="0.5" start="0"/>',\
 '<head id="action" type="shake" amount="0.2" repeats="1" start="anticipation:relax"/>',\
 '<head id="overshoot" type="shake" amount="0.03" repeats="0.5" start="action:relax" />',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'small_shake_closed_eyes',\
('<face id="heavy_eyelid" type="facs" au="45" amount="0.5" start="0" ready="0.5" end="1"/>  ',\
 '<head id="anticipation" type="shake" amount="-0.03" repeats="0.5" start="0"/>',\
 '<head id="action" type="shake" amount="0.1" repeats="1" start="anticipation:relax"/>',\
 '<head id="overshoot" type="shake" amount="0.03" repeats="0.5" start="action:relax" />',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'shake_twice_closed_eyes',\
('<face id="heavy_eyelid" type="facs" au="5" amount="-0.3" start="0" ready="0.5" end="1"/>  ',\
 '<head id="anticipation" type="shake" amount="-0.03" repeats="0.5" start="0"/>',\
 '<head id="action" type="shake" amount="0.1" repeats="2" velocity="1.6" start="anticipation:relax"/>',\
 '<head id="overshoot" type="shake" amount="0.03" repeats="0.5" start="action:relax" />',))
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
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'head_to_right',\
('<head type="ORIENT" amount="0.15" angle="0.5" direction="RIGHT" relax="$relax_time" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'head_to_left',\
('<head type="ORIENT" amount="0.15" angle="0.5" direction="LEFT" relax="$relax_time" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'head_up',\
('<head type="ORIENT" amount="0.15" angle="0.5" direction="UP" relax="$relax_time" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'head_down',\
('<head type="ORIENT" amount="0.15" angle="0.5" direction="DOWN" relax="$relax_time" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'head_tilt_right',\
('<head type="ORIENT" amount="0.15" angle="0.5" direction="ROLLRIGHT" relax="$relax_time" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'head_tilt_left',\
('<head type="ORIENT" amount="0.15" angle="0.5" direction="ROLLLEFT" relax="$relax_time" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'brow_frown',\
('<face type="facs" au="4" amount="0.5" relax="$ready_time" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'brow_raise',\
('<face type="facs" au="2" amount="0.5" relax="$ready_time" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'smile',\
('<face amount="0.7" au="12" end="$relax_time" side="BOTH" start="0.0" type="facs"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_right',\
('<gesture stroke="0" priority="3" mode="RIGHT_HAND" lexeme="RIGHT""/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_left',\
('<gesture ready="$start_time" stroke="0" priority="3" mode="LEFT_HAND" lexeme="LEFT"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gesture_chop',\
('<gesture ready="$start_time" stroke="0" priority="3" mode="RIGHT_HAND" lexeme="CHOP"/>',))
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
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_aversion_downleft',\
('<gaze target="$target" direction="DOWNLEFT" angle="20"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_aversion_upleft',\
('<gaze target="$target" direction="UPLEFT" angle="20"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_aversion_upright',\
('<gaze target="$target" direction="UPRIGHT" angle="20"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_aversion_upleft',\
('<gaze target="$target" direction="DOWNRIGHT" angle="20"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_cursory',\
('<gaze target="$target" direction="POLAR 0" angle="0" sbm:joint-range="NECK EYES" start="$ready_time" end="" priority="3"/>',\
 '<gaze target="$prev_target" direction="POLAR 0" angle="0" sbm:joint-range="NECK EYES" start="$ready_time+1.5" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_saccade',\
('<gaze target="$target" direction="RIGHT" angle="5" sbm:joint-range="EYES" start="$ready_time" priority="3"/>',\
 '<gaze target="$target" direction="LEFT" angle="5" sbm:joint-range="EYES" start="$ready_time+0.5" priority="3"/>',\
 '<gaze target="$target" direction="POLAR 0" angle="0" sbm:joint-range="EYES" start="$ready_time+1.0" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'atd_nod',\
('<head id="atd_h1" type="NOD" amount="1" repeats="0.5" velocity="3.5" sbm:smooth="0.35" start="0.0" ready="0.2" relax="0.28" end="0.7" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'atd_gaze',\
('<gaze id="atd_g1" target="$target" sbm:roll="3" sbm:joint-range="NECK HEAD EYES" sbm:joint-speed="300 300 80" priority="3"/>',\
 '<sbm:head pitch="-5" heading="-8"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'understanding',\
('<head id="ud_h1" type="NOD" amount="1" repeats="1" velocity="3" sbm:smooth="0.6" start="0.0" ready="0.7" relax="1.0" end="2.0" priority="3"/>',\
 '<face id="agree_f1" type="facs" au="1" amount="0.2" sbm:rampup="0.33" sbm:rampdown="0.4" sbm:duration="1.0" start="ud_h1:start" end="ud_h1:end" priority="3"/>',\
 '<face id="agree_f2" type="facs" au="2" amount="0.6" sbm:rampup="0.33" sbm:rampdown="0.4" sbm:duration="1.0" start="agree_f1:start" end="agree_f1:end" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'atd_glance',\
('<gaze id="atd_gl1" target="$target" sbm:roll="3" sbm:joint-range="NECK HEAD EYES" sbm:joint-speed="950 1000 340" priority="3" start="0.0" end="0.8"/>',\
 '<gaze id="atd_gl2" target="$prev_target" sbm:joint-range="NECK HEAD EYES" sbm:joint-speed="800 800 300" priority="3" start="atd_gl1:end+0.5" end="atd_gl1:end+1.5"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gi_glance',\
('<gaze id="gi_gl1" target="$target" sbm:roll="2" sbm:joint-range="NECK HEAD EYES" sbm:joint-speed="950 1000 340" priority="3" start="0.0" end="0.8"/>',\
 '<gaze id="gi_gl2" target="$target" sbm:joint-range="EYES" sbm:joint-speed="700 700 100" priority="3" start="gi_gl1:end+0.2" end="gi_gl1:end+0.23"/>',\
 '<gaze id="gi_gl3" target="$target" sbm:joint-range="EYES" sbm:joint-speed="700 700 100" priority="3" start="gi_gl2:end" end="gi_gl2:end+0.03"/>',\
 '<gaze id="gi_gl4" target="$target" sbm:joint-range="EYES" sbm:joint-speed="700 700 50" priority="3" start="gi_gl3:end+0.2" end="gi_gl3:end+0.45"/>',\
 '<gaze id="gi_gl5" target="$target" sbm:joint-range="NECK HEAD EYES" sbm:joint-speed="700 700 200" priority="3" start="gi_gl4:end+0.5" end="gi_gl4:end+1.5"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gi_furtive_glance',\
('<gaze id="gi_fgl1" target="$target" sbm:joint-range="EYES" sbm:joint-speed="700 700 200" priority="3" start="0.0" end="0.3"/>',\
 '<gaze id="gi_fgl2" target="$target" sbm:joint-range="EYES" sbm:joint-speed="700 700 200" priority="3" direction="DOWNRIGHT" angle="3" start="gi_fgl1:end" end="gi_fgl1:end+0.03"/>',\
 '<gaze id="gi_fgl3" target="$target" sbm:joint-range="EYES" sbm:joint-speed="700 700 200" priority="3" direction="DOWN" angle="5" start="gi_fgl2:end" end="gi_fgl2:end+0.2"/>',\
 '<gaze id="gi_fgl4" target="$target" sbm:joint-range="EYES" sbm:joint-speed="700 700 200" priority="3" start="gi_fgl3:end" end="gi_fgl3:end+0.3"/>',\
 '<gaze id="gi_fgl5" target="$prev_target" sbm:joint-range="EYES" sbm:joint-speed="700 700 200" priority="3" start="gi_gl4+0.05" end="gi_gl4+0.4"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'thk_gaze_cursory',\
('<gaze id="thk_gl1" target="$target" sbm:roll="-5" sbm:joint-range="HEAD EYES" sbm:joint-speed="700 700 200" direction="UP" angle="3" priority="3" start="0.0" end="0.3"/>',\
 '<sbm:eyes pitch="-2" heading="-10"/>',\
 '<sbm:head pitch="2" heading="-8"/>',\
 '<gaze id="thk_gl2" target="$target" sbm:joint-range="EYES" direction="POLAR 45" angle="10" priority="3" start="thk_gl1:end" end="thk_gl1:end+0.2"/>',\
 '<gaze id="thk_gl3" target="$target" sbm:joint-range="EYES" direction="POLAR 60" angle="15" priority="3" start="thk_gl2:end" end="thk_gl2:end+0.2"/>',\
 '<gaze id="thk_gl4" target="$target" sbm:joint-range="EYES" direction="POLAR 45" angle="7" priority="3" start="thk_gl3:end+0.5" end="thk_gl3:end+0.8"/>',\
 '<sbm:eyes heading="-8"/>',\
 '<gaze id="thk_gl5" target="$target" sbm:joint-range="EYES" direction="POLAR 60" angle="15" priority="3" start="thk_gl4:end" end="thk_gl4:end+0.2"/>',\
 '<gaze id="thk_gl6" target="$target" sbm:joint-range="EYES" direction="POLAR 80" angle="15" priority="3" start="thk_gl5:end+0.5" end="thk_gl5:end+0.8"/>',\
 '<gaze id="thk_gl7" target="$target" sbm:joint-range="HEAD EYES" sbm:joint-speed="50 50 40" priority="3" start="thk_gl6:end+1" end="thk_gl6:end+1.5"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'partial_understand',\
('<head id="pu_h1" type="NOD" repeats="1" amount="0.5" velocity="3" start="0" ready="0.7" relax="1.0" end="2.0" sbm:smooth="0.6"/>',\
 '<face id="pu_f1" type="facs" au="4" amount="0.5" start="pu_h1:start" relax="pu_h1:relax" sbm:rampup="0.2" sbm:rampdown="0.4"/>',\
 '<gaze id="pu_g1" target="$target" sbm:roll="-5" sbm:joint-range="HEAD EYES" sbm:joint-speed="50 50 20" angle="3" priority="3" start="pu_f1:start" end="pu_f1:relax"/>',\
 '<sbm:head pitch="-5"/>',\
 '<gaze id="pu_g2" target="$target" sbm:roll="-5" sbm:joint-range="HEAD EYES" sbm:joint-speed="50 50 20" angle="3" priority="3" start="pu_g1:end+0.2" relax="pu_f1:end"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'confusion',\
('<face id="cfs_f1" type="facs" au="4" amount="0.5" start="0" relax="1.8" sbm:rampup="0.2" sbm:rampdown="0.4"/>',\
 '<head id="cfs_h1" type="SHAKE" repeats="1" amount="0.5" velocity="3" start="cfs_f1:ready" ready="cfs_f1:ready+0.75" relax="cfs_f1:ready+0.8" end="cfs_f1:ready+1" sbm:smooth="0.85"/>',\
 '<head id="cfs_h2" type="SHAKE" repeats="1" amount="0.15" velocity="2.5" start="cfs_h1:end" ready="cfs_f1:end+0.15" relax="cfs_f1:end+0.3" end="cfs_f1:end+5" sbm:smooth="0.85"/>',\
 '<gaze id="cfs_g1" target="$target" sbm:roll="-5" sbm:joint-range="HEAD EYES" sbm:joint-speed="50 50 20" angle="3" priority="3" start="cfs_f1:start" end="cfs_f1:relax"/>',\
 '<sbm:head pitch="-5"/>',\
 '<gaze id="cfs_g2" target="$target" sbm:roll="-5" sbm:joint-range="HEAD EYES" sbm:joint-speed="50 50 20" angle="3" priority="3" start="cfs_g1:end+0.2" relax="cfs_f1:end"/>',))
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
('<head id="agree_h1" type="NOD" amount="0.8" repeats="1.5" velocity="3" sbm:smooth="0.7" start="0.0" ready="0.25" relax="0.8" end="1.5" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'attitude-disagree',\
('<gaze id="disagree_g1" target="$target" start="0.0" end="0.5"/>',\
 '<head id="disagree_h1" type="SHAKE" repeats="1" amount="0.55" velocity="1" sbm:smooth="0.4" start="disagree_g1:end"/>',\
 '<head id="disagree_h2" type="SHAKE" repeats="1" amount="0.4" velocity="1.5" sbm:smooth="0.7" start="disagree_h1:end-0.1"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'attitude-like',\
('<face id="like_f1" type="facs" au="12" amount="1" sbm:rampup="0.6" sbm:rampdown="1" start="0" ready="0.8" relax="2.0" end="3.0" priority="3"/>',\
 '<face id="like_f2" type="facs" au="6" amount="0.2" sbm:rampup="0.2" sbm:rampdown="1.03" start="like_f1:start-0.05" ready="like_f1:ready" relax="like_f1:relax" end="like_f1:end+0.03" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_focus',\
('<gaze id="focus_g1" target="$target" direction="POLAR 0" angle="0" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_weak_focus',\
('<gaze id="weakfocus_g1" target="$target" direction="POLAR 0" angle="0" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_look',\
('<gaze id="weakfocus_g1" target="$target" direction="POLAR 0" angle="0" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_cursory_2',\
('<gaze target="$target" direction="POLAR 0" angle="0" sbm:joint-range="HEAD EYES" start="0.0" priority="3"/>',\
 '<gaze target="$prev_target" direction="POLAR 0" angle="0" sbm:joint-range="HEAD EYES" start="1.5" priority="3"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'gaze_quick_avert',\
('<gaze target="alesia" id="avert" direction="DOWNLEFT" angle="12" sbm:joint-range="EYES" sbm:joint-speed="100" start="0" end="0.4"/>',\
 '<gaze target="alesia" id="avert-ret" sbm:joint-range="EYES" sbm:joint-speed="100" start="0.6"/>',))
UniversalFactUtil.addDefaultBehaviorMapping(nvbg_engine,'headtiltleft',\
('<head sbm:state-name="paramHeadTilt" stroke="0" relax="$end" type="PARAMETERIZED" x="-30" y="0" />',))
