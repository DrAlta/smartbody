include_directories( "." )
include_directories( "../../core/smartbody/sbm-debugger/lib" )
include_directories( "../../lib/festival/src/include" )
include_directories( "../../lib/festival/src/modules/VHDuration" )
include_directories( "../../lib/speech_tools/include" )


####

set( SMARTBODY_LIB_EXT_ANN_SRC
	
	external/ANN/ANN.cpp
	external/ANN/bd_fix_rad_search.cpp
	external/ANN/bd_pr_search.cpp
	external/ANN/bd_search.cpp
	external/ANN/bd_tree.cpp
	external/ANN/brute.cpp
	external/ANN/kd_dump.cpp
	external/ANN/kd_fix_rad_search.cpp
	external/ANN/kd_pr_search.cpp
	external/ANN/kd_search.cpp
	external/ANN/kd_split.cpp
	external/ANN/kd_tree.cpp
	external/ANN/kd_util.cpp
	external/ANN/perf.cpp

)

set( SMARTBODY_LIB_EXT_PARSER_SRC
	
	external/parser/Bchart.cpp
	external/parser/BchartSm.cpp
	external/parser/Bst.cpp
	external/parser/ChartBase.cpp
	external/parser/ClassRule.cpp
	external/parser/CntxArray.cpp
	external/parser/CombineBests.cpp
	external/parser/ECArgs.cpp
	external/parser/Edge.cpp
	external/parser/EdgeHeap.cpp
	external/parser/edgeSubFns.cpp
	external/parser/EgsFromTree.cpp
	external/parser/ewDciTokStrm.cpp
	external/parser/extraMain.cpp
	external/parser/FBinaryArray.cpp
	external/parser/Feat.cpp
	external/parser/Feature.cpp
	external/parser/FeatureTree.cpp
	external/parser/fhSubFns.cpp
	external/parser/Field.cpp
	external/parser/FullHist.cpp
	external/parser/GotIter.cpp
	external/parser/headFinder.cpp
	external/parser/headFinderCh.cpp
	external/parser/InputTree.cpp
	external/parser/Item.cpp
	external/parser/Link.cpp
	external/parser/MeChart.cpp
	external/parser/Params.cpp
	external/parser/ParseStats.cpp
	external/parser/SentRep.cpp
	external/parser/Term.cpp
	external/parser/TimeIt.cpp
	external/parser/UnitRules.cpp
	external/parser/utils.cpp
	external/parser/ValHeap.cpp

)

set( SMARTBODY_LIB_EXT_SOIL_SRC
	
	external/SOIL/image_DXT.c
	external/SOIL/image_helper.c
	external/SOIL/SOIL.c
	external/SOIL/stb_image_aug.c
#	external/SOIL/stbi_DDS_aug_c.h
)
set( SMARTBODY_LIB_EXT_PERLIN_SRC
	
	external/perlin/perlin.cpp
)
set( SMARTBODY_LIB_ME_SRC

	controllers/me_controller_context.cpp
	controllers/me_controller_context_proxy.cpp
	controllers/me_controller.cpp
	controllers/me_controller_tree_root.cpp
	controllers/me_ct_adshr_envelope.cpp
	controllers/me_ct_blend.cpp
	controllers/me_ct_channel_writer.cpp
	controllers/me_ct_container.cpp
	controllers/me_ct_curve_writer.cpp
	controllers/me_ct_interpolator.cpp
	controllers/me_ct_lifecycle_test.cpp
	controllers/me_ct_motion.cpp
	controllers/me_ct_periodic_replay.cpp
	controllers/me_ct_pose.cpp
	controllers/me_ct_scheduler2.cpp
	controllers/me_ct_time_shift_warp.cpp
	controllers/me_ct_unary.cpp
	controllers/me_default_prune_policy.cpp
	controllers/me_spline_1d.cpp
)

set( SMARTBODY_LIB_SK_SRC

	sk/sk_channel_array.cpp
	sk/sk_channel.cpp
	sk/sk_joint.cpp
	sk/sk_joint_euler.cpp
	sk/sk_joint_pos.cpp
	sk/sk_joint_quat.cpp
	sk/sk_joint_swing_twist.cpp
	sk/sk_motion.cpp
	sk/sk_motion_io.cpp
	sk/sk_posture.cpp
	sk/sk_scene.cpp
	sk/sk_skeleton.cpp
	sk/sk_skeleton_io.cpp
	sk/sk_vec_limits.cpp
)

set( SMARTBODY_LIB_SR_SRC

	sr/sr_alg.cpp
	sr/sr_array.cpp
	sr/sr_box.cpp
	sr/sr_buffer.cpp
	sr/sr_camera.cpp
	sr/sr_color.cpp
	sr/sr.cpp
	sr/sr_cylinder.cpp
	sr/sr_euler.cpp
	sr/sr_event.cpp
	sr/sr_geo2.cpp
	sr/sr_gl.cpp
	sr/sr_gl_render_funcs.cpp
	sr/sr_hash_table.cpp
	sr/sr_input.cpp
	sr/sr_light.cpp
	sr/sr_line.cpp
	sr/sr_lines.cpp
	sr/sr_mat.cpp
	sr/sr_material.cpp
	sr/sr_model.cpp
	sr/sr_model_export_iv.cpp
	sr/sr_model_import_obj.cpp
	sr/sr_output.cpp
	sr/sr_path_array.cpp
	sr/sr_plane.cpp
	sr/sr_points.cpp
	sr/sr_polygon.cpp
	sr/sr_polygons.cpp
	sr/sr_quat.cpp
	sr/sr_sa_bbox.cpp
	sr/sr_sa.cpp
	sr/sr_sa_event.cpp
	sr/sr_sa_gl_render.cpp
	sr/sr_sa_render_mode.cpp
	sr/sr_shared_ptr.cpp
	sr/sr_sn.cpp
	sr/sr_sn_editor.cpp
	sr/sr_sn_group.cpp
	sr/sr_sn_matrix.cpp
	sr/sr_sn_shape.cpp
	sr/sr_sn_colorsurf.cpp
	sr/sr_sphere.cpp
	sr/sr_spline.cpp
	sr/sr_string_array.cpp
	sr/sr_string.cpp
	sr/sr_timer.cpp
	sr/sr_trackball.cpp
	sr/sr_tree.cpp
	sr/sr_triangle.cpp
	sr/sr_vec2.cpp
	sr/sr_vec.cpp
	sr/sr_viewer.cpp
	sr/planner/sk_blend_cfg.cpp
	sr/planner/sk_pos_planner.cpp
	sr/planner/sk_blend_cfg_manager.cpp
	sr/planner/sr_cfg_planner.cpp
	sr/planner/sk_blend_planner.cpp
	sr/planner/sr_cfg_manager.cpp
	sr/planner/sk_pos_cfg.cpp
	sr/planner/sr_cfg_planner_lazy.cpp
	sr/planner/sr_cfg_path.cpp
	sr/planner/sr_cfg_tree.cpp
	sr/planner/sk_pos_cfg_manager.cpp
)

set( SMARTBODY_LIB_SBM_SRC

	sbm/BMLDefs.cpp
	bml/behavior_scheduler_constant_speed.cpp
	bml/behavior_scheduler.cpp
	bml/behavior_scheduler_fixed.cpp
	bml/behavior_span.cpp
	bml/bml_animation.cpp
	bml/bml_gesture.cpp
	bml/bml_bodyreach.cpp
	bml/bml_constraint.cpp
	bml/bml.cpp
	bml/bml_event.cpp
	bml/bml_face.cpp
	bml/bml_gaze.cpp
	bml/bml_general_param.cpp
	bml/bml_grab.cpp
	bml/bml_interrupt.cpp
	bml/bml_locomotion.cpp
	bml/bml_processor.cpp
	bml/bml_quickdraw.cpp
	bml/bml_reach.cpp
	bml/bml_speech.cpp
	bml/bml_sync_point.cpp
	bml/bml_target.cpp
	bml/bml_saccade.cpp
	bml/bml_noise.cpp
	bml/bml_states.cpp
	sbm/Event.cpp
	sbm/GenericViewer.cpp
	sbm/gwiz_cmdl.cpp
	sbm/gwiz_math.cpp
	sbm/gwiz_spline.cpp
	sbm/Heightfield.cpp
	sbm/joint_logger.cpp
	sbm/lin_win.cpp
	sbm/mcontrol_callbacks.cpp
	sbm/mcontrol_util.cpp
	sbm/action_unit.cpp
	
	controllers/me_ct_basic_locomotion.cpp
	controllers/me_ct_ccd_IK.cpp
	controllers/me_ct_constraint.cpp
	controllers/me_ct_data_driven_reach.cpp
	controllers/me_ct_data_interpolation.cpp
	controllers/me_ct_example_body_reach.cpp
	controllers/me_ct_examples.cpp
	controllers/me_ct_eyelid.cpp
	controllers/me_ct_face.cpp
	controllers/me_ct_gaze_alg.cpp
	controllers/me_ct_gaze.cpp
	controllers/me_ct_gaze_joint.cpp
	controllers/me_ct_gaze_keymap.cpp
	controllers/me_ct_gaze_target.cpp
	controllers/me_ct_IK.cpp
	controllers/me_ct_IK_scenario.cpp
	controllers/me_ct_jacobian_IK.cpp
	controllers/me_ct_limb.cpp
	controllers/me_ct_locomotion_func.cpp
	controllers/me_ct_motion_example.cpp
	controllers/me_ct_motion_parameter.cpp
	controllers/me_ct_motion_player.cpp
	controllers/me_ct_motion_timewarp.cpp
	controllers/me_ct_param_animation.cpp
	controllers/me_ct_param_animation_data.cpp
	controllers/me_ct_param_animation_utilities.cpp
	controllers/me_ct_quick_draw.cpp
	controllers/me_ct_reach.cpp
	controllers/me_ct_hand.cpp
	controllers/me_ct_motion_profile.cpp
	controllers/me_ct_barycentric_interpolation.cpp
	controllers/me_ct_inverse_interpolation.cpp
	controllers/me_ct_reach_IK.cpp
	controllers/me_ct_simple_gaze.cpp
	controllers/me_ct_tether.cpp
	controllers/me_ct_ublas.cpp
	sbm/me_utilities.cpp
	controllers/me_ct_breathing.cpp
	controllers/me_ct_breathing_interface.cpp
	controllers/me_ct_physics_controller.cpp
	controllers/me_ct_noise_controller.cpp
	controllers/me_ct_motion_recorder.cpp
	
	controllers/MeCtBlendEngine.cpp
	controllers/me_ct_motion_blend.cpp
	controllers/MotionAnalysis.cpp

	sbm/ParserBVH.cpp
	sbm/ParserASFAMC.cpp
	sbm/ParserFBX.cpp
	sbm/ParserOpenCOLLADA.cpp
	sbm/remote_speech.cpp
	sbm/resource_cmds.cpp
	sbm/Resource.cpp
	sbm/ResourceManager.cpp
	sbm/sbm_audio.cpp
	controllers/MeCtBodyReachState.cpp
	controllers/MeCtReachEngine.cpp
	controllers/me_ct_saccade.cpp
	sbm/local_speech.cpp

## UNUSED:
##	sbm/sbm_bml.cpp
	sbm/sbm_character.cpp
	sbm/sbm_deformable_mesh.cpp
	sbm/sbm_pawn.cpp
	sbm/sbm_speech_audiofile.cpp
	sbm/sbm_speech.cpp
	sbm/sbm_speech_impl_skeleton.cpp
	sbm/sbm_test_cmds.cpp
	sbm/sr_arg_buff.cpp
	sbm/sr_cmd_line.cpp
	sbm/sr_cmd_seq.cpp
	sbm/sr_hash_map.cpp
	sbm/sr_linear_curve.cpp
	sbm/sr_spline_curve.cpp
	sbm/sr_synch_points.cpp
	sbm/text_speech.cpp
	sbm/time_profiler.cpp
	sbm/time_regulator.cpp
	sbm/xercesc_utils.cpp
	
	sbm/Physics/SbmColObject.cpp
	sbm/Physics/SbmColObject.cpp
	sbm/Physics/SbmPhysicsSim.cpp
	sbm/Physics/SbmPhysicsSimODE.cpp

	sbm/GPU/SbmTexture.cpp
	sbm/GPU/SbmDeformableMeshGPU.cpp
	sbm/GPU/TBOData.cpp
	sbm/GPU/VBOData.cpp
	sbm/GPU/SbmShader.cpp
	sbm/SteeringAgent.cpp
	sbm/SteerSuiteEnginerDriver.cpp
	sbm/sr_path_list.cpp

	sb/SBAttribute.cpp
	sb/SBAttributeManager.cpp
	sb/SBObject.cpp
	sb/SBObserver.cpp
	sb/SBSubject.cpp
	sbm/DefaultAttributeTable.cpp

	sb/SBScene.cpp
	sbm/KinectProcessor.cpp
	controllers/me_ct_data_receiver.cpp

	sb/SBScript.cpp
	sb/SBCharacter.cpp
	sb/SBPawn.cpp
	sb/SBJoint.cpp
	sb/SBSkeleton.cpp
	sb/SBController.cpp
	sb/SBSimulationManager.cpp
	sb/SBBmlProcessor.cpp
	sb/SBAnimationState.cpp
	sb/SBAnimationTransition.cpp
	sb/SBAnimationStateManager.cpp
	sb/SBSteerManager.cpp
	sb/SBSteerAgent.cpp
	sb/SBReachManager.cpp
	sb/SBReach.cpp
	sb/SBGestureMapManager.cpp
	sb/SBGestureMap.cpp
	sb/SBServiceManager.cpp
	sb/SBService.cpp
	sb/SBMotion.cpp
	sb/SBFaceDefinition.cpp
	sb/SBPhysicsManager.cpp
	sb/SBBoneBusManager.cpp
	sb/SBCollisionManager.cpp
	sb/SBParser.cpp
	sb/SBParseNode.cpp
	sb/SBJointMap.cpp
	sb/SBJointMapManager.cpp
	sb/SBPhoneme.cpp
	sb/SBPhonemeManager.cpp
	sb/SBBehaviorSet.cpp
	sb/SBBehaviorSetManager.cpp
	sb/SBMotionBlendBase.cpp

	sb/SBPython.cpp
	sb/SBPythonClass.cpp

	sbm/nvbg.cpp
	sbm/MiniBrain.cpp
	sb/SBBehavior.cpp
	sbm/SbmDebuggerServer.cpp
	sbm/SbmDebuggerClient.cpp
	sbm/SbmDebuggerUtility.cpp
	sbm/SteerPath.cpp

)

####


#set( CMAKE_MODULE_LINKER_FLAGS -m32 )
set_source_files_properties( ${SMARTBODY_LIB_EXT_SOIL_SRC} PROPERTIES LANGUAGE CXX )

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	if (${CMAKE_C_COMPILER_VERSION} VERSION_GREATER 4.2.0)
		add_definitions(-Wno-deprecated-writable-strings)
	endif()
endif()


add_library(
	smartbody-lib
	STATIC 
	
	${SMARTBODY_LIB_EXT_ANN_SRC}

	${SMARTBODY_LIB_EXT_SOIL_SRC}
	
	${SMARTBODY_LIB_EXT_PARSER_SRC}

	${SMARTBODY_LIB_EXT_PERLIN_SRC}
	
 	${SMARTBODY_LIB_ME_SRC}
	
 	${SMARTBODY_LIB_SK_SRC}
	
	${SMARTBODY_LIB_SR_SRC}

	${SMARTBODY_LIB_SBM_SRC}
)
