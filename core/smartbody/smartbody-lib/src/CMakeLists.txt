include_directories( "." )

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

set( SMARTBODY_LIB_EXT_SOIL_SRC
	
	external/SOIL/image_DXT.c
	external/SOIL/image_helper.c
	external/SOIL/SOIL.c
	external/SOIL/stb_image_aug.c
	external/SOIL/stbi_DDS_aug_c.h
)

set( SMARTBODY_LIB_ME_SRC

	me/me_controller_context.cpp
	me/me_controller_context_proxy.cpp
	me/me_controller.cpp
	me/me_controller_tree_root.cpp
	me/me_ct_adshr_envelope.cpp
	me/me_ct_blend.cpp
	me/me_ct_channel_writer.cpp
	me/me_ct_container.cpp
	me/me_ct_curve_writer.cpp
	me/me_ct_interpolator.cpp
	me/me_ct_lifecycle_test.cpp
	me/me_ct_motion.cpp
	me/me_ct_periodic_replay.cpp
	me/me_ct_pose.cpp
	me/me_ct_scheduler2.cpp
	me/me_ct_time_shift_warp.cpp
	me/me_ct_unary.cpp
	me/me_default_prune_policy.cpp
	me/me_spline_1d.cpp
)

set( SMARTBODY_LIB_SK_SRC

	sk/sk_channel_array.cpp
	sk/sk_channel.cpp
	sk/sk_joint.cpp
	sk/sk_joint_euler.cpp
	sk/sk_joint_name.cpp
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
	sbm/behavior_scheduler_constant_speed.cpp
	sbm/behavior_scheduler.cpp
	sbm/behavior_scheduler_fixed.cpp
	sbm/behavior_span.cpp
	sbm/bml_animation.cpp
	sbm/bml_bodyreach.cpp
	sbm/bml_constraint.cpp
	sbm/bml.cpp
	sbm/bml_event.cpp
	sbm/bml_face.cpp
	sbm/bml_gaze.cpp
	sbm/bml_general_param.cpp
	sbm/bml_grab.cpp
	sbm/bml_interrupt.cpp
	sbm/bml_locomotion.cpp
	sbm/bml_processor.cpp
	sbm/bml_quickdraw.cpp
	sbm/bml_reach.cpp
	sbm/bml_speech.cpp
	sbm/bml_sync_point.cpp
	sbm/bml_target.cpp
	sbm/bml_saccade.cpp
	sbm/bml_states.cpp
	sbm/BoneMap.cpp
	sbm/Event.cpp
	sbm/GenericViewer.cpp
	sbm/gwiz_cmdl.cpp
	sbm/gwiz_math.cpp
	sbm/gwiz_spline.cpp
	sbm/Heightfield.cpp
	sbm/joint_logger.cpp
	sbm/lin_win.cpp
	sbm/locomotion_cmds.cpp
	sbm/mcontrol_callbacks.cpp
	sbm/mcontrol_util.cpp
	sbm/action_unit.cpp
	
	sbm/me_ct_basic_locomotion.cpp
	sbm/me_ct_ccd_IK.cpp
	sbm/me_ct_constraint.cpp
	sbm/me_ct_data_driven_reach.cpp
	sbm/me_ct_data_interpolation.cpp
	sbm/me_ct_example_body_reach.cpp
	sbm/me_ct_examples.cpp
	sbm/me_ct_eyelid.cpp
	sbm/me_ct_face.cpp
	sbm/me_ct_gaze_alg.cpp
	sbm/me_ct_gaze.cpp
	sbm/me_ct_gaze_joint.cpp
	sbm/me_ct_gaze_keymap.cpp
	sbm/me_ct_gaze_target.cpp
	sbm/me_ct_IK.cpp
	sbm/me_ct_IK_scenario.cpp
	sbm/me_ct_jacobian_IK.cpp
	sbm/me_ct_lilt_try.cpp
	sbm/me_ct_limb.cpp
	sbm/me_ct_locomotion_analysis.cpp
	sbm/me_ct_locomotion_anim_global_info.cpp
	sbm/me_ct_locomotion_balance.cpp
	sbm/me_ct_locomotion.cpp
	sbm/me_ct_locomotion_func.cpp
	sbm/me_ct_locomotion_height_offset.cpp
	sbm/me_ct_locomotion_IK.cpp
	sbm/me_ct_locomotion_joint_info.cpp
	sbm/me_ct_locomotion_limb_anim.cpp
	sbm/me_ct_locomotion_limb.cpp
	sbm/me_ct_locomotion_limb_direction_planner.cpp
	sbm/me_ct_locomotion_navigator.cpp
	sbm/me_ct_locomotion_pawn.cpp
	sbm/me_ct_locomotion_quadratic_synchronizer.cpp
	sbm/me_ct_locomotion_routine.cpp
	sbm/me_ct_locomotion_simple.cpp
	sbm/me_ct_locomotion_speed_accelerator.cpp
	sbm/me_ct_locomotion_terrain.cpp
	sbm/me_ct_locomotion_timing_space.cpp
	sbm/me_ct_motion_example.cpp
	sbm/me_ct_motion_parameter.cpp
	sbm/me_ct_motion_player.cpp
	sbm/me_ct_motion_timewarp.cpp
	sbm/me_ct_navigation_circle.cpp
	sbm/me_ct_navigation_waypoint.cpp
	sbm/me_ct_param_animation.cpp
	sbm/me_ct_param_animation_data.cpp
	sbm/me_ct_param_animation_utilities.cpp
	sbm/me_ct_quick_draw.cpp
	sbm/me_ct_reach.cpp
	sbm/me_ct_hand.cpp
	sbm/me_ct_motion_profile.cpp
	sbm/me_ct_barycentric_interpolation.cpp
	sbm/me_ct_inverse_interpolation.cpp
	sbm/me_ct_reach_IK.cpp
	sbm/me_ct_simple_gaze.cpp
	sbm/me_ct_step_turn.cpp
	sbm/me_ct_tether.cpp
	sbm/me_ct_ublas.cpp
	sbm/me_utilities.cpp
	sbm/me_ct_breathing.cpp
	sbm/me_ct_breathing_interface.cpp
	sbm/ParserBVH.cxx
	sbm/ParserASFAMC.cxx
	sbm/ParserFBX.cpp
	sbm/ParserOpenCOLLADA.cpp
	sbm/remote_speech.cpp
	sbm/resource_cmds.cpp
	sbm/Resource.cpp
	sbm/ResourceManager.cpp
	sbm/sbm_audio.cpp
	sbm/MeCtBodyReachState.cpp
	sbm/MeCtReachEngine.cpp
	sbm/me_ct_saccade.cpp
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

	sbm/DAttribute.cpp
	sbm/DAttributeManager.cpp
	sbm/DObject.cpp
	sbm/DObserver.cpp
	sbm/DSubject.cpp
	sbm/DefaultAttributeTable.cpp

	sbm/SBScene.cpp
	sbm/KinectProcessor.cpp
	sbm/me_ct_data_receiver.cpp

	sbm/SBCharacter.cpp
	sbm/SBJoint.cpp
	sbm/SBSkeleton.cpp
	sbm/SBController.cpp
	sbm/VisemeMap.cpp
	sbm/sr_path_list.cpp

	sbm/SbmPython.cpp
	sbm/SbmPythonClass.cpp

	sbm/nvbg.cpp
	sbm/MiniBrain.cpp
	sbm/SBBehavior.cpp

)

####

#set( CMAKE_MODULE_LINKER_FLAGS -m32 )

add_library(
	smartbody-lib
	STATIC 
	
	${SMARTBODY_LIB_EXT_ANN_SRC}

	${SMARTBODY_LIB_EXT_SOIL_SRC}
	
 	${SMARTBODY_LIB_ME_SRC}
	
 	${SMARTBODY_LIB_SK_SRC}
	
	${SMARTBODY_LIB_SR_SRC}

	${SMARTBODY_LIB_SBM_SRC}
)
