# log4cxx
#
#  Files not yet added to the build :
#  1. these files depend on the GL/GLU/GLEW. Need to figure out a way to build them with OpenGL ES
#	$(SBM_MY_DIR)/sr/sr_gl.cpp \
#	$(SBM_MY_DIR)/sr/sr_gl_render_funcs.cpp \
#   
#   $(SBM_MY_DIR)/sr/sr_sa_gl_render.cpp \
#	$(SBM_MY_DIR)/sbm/GPU/SbmTexture.cpp \
#	$(SBM_MY_DIR)/sbm/GPU/SbmDeformableMeshGPU.cpp \
#	$(SBM_MY_DIR)/sbm/GPU/TBOData.cpp \
#	$(SBM_MY_DIR)/sbm/GPU/VBOData.cpp \
#	$(SBM_MY_DIR)/sbm/GPU/SbmShader.cpp 
#  2. Unused or to be removed soon
#   $(SBM_MY_DIR)/sbm/me_ct_reach.cpp \
#   $(SBM_MY_DIR)/sbm/bml_reach.cpp \
#   $(SBM_MY_DIR)/sbm/VisemeMap.cpp \


SBM_LOCAL_PATH := $(call my-dir)
LOCAL_PATH := $(SBM_LOCAL_PATH)
SBM_MY_DIR := ../../../core/smartbody/smartbody-lib/src
ANDROID_LIB_DIR := ../../lib
LIB_DIR := ../../../lib

include $(CLEAR_VARS)
LOCAL_MODULE := xerces-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libxerces-c.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := boost-filesystem-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libboost_filesystem.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := boost-system-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libboost_system.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := boost-regex-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libboost_regex.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := lapack
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/lapack_ANDROID.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := blas
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/blas_ANDROID.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := f2c
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libf2c_ANDROID.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := iconv-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libiconv.a
include $(PREBUILT_STATIC_LIBRARY)

#include $(CLEAR_VARS)
#LOCAL_MODULE := python-prebuilt
#LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libpython2.6.so
#include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := python-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libpython2.6.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := boost-python-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libboost_python.a
LOCAL_STATIC_LIBRARIES := python-prebuilt	
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := ann
LOCAL_CFLAGS    := -DBUILD_ANDROID
LOCAL_SRC_FILES := $(SBM_MY_DIR)/external/ANN/ANN.cpp \
	$(SBM_MY_DIR)/external/ANN/bd_fix_rad_search.cpp \
	$(SBM_MY_DIR)/external/ANN/bd_pr_search.cpp \
	$(SBM_MY_DIR)/external/ANN/bd_search.cpp \
	$(SBM_MY_DIR)/external/ANN/bd_tree.cpp \
	$(SBM_MY_DIR)/external/ANN/brute.cpp \
	$(SBM_MY_DIR)/external/ANN/kd_dump.cpp \
	$(SBM_MY_DIR)/external/ANN/kd_fix_rad_search.cpp \
	$(SBM_MY_DIR)/external/ANN/kd_pr_search.cpp \
	$(SBM_MY_DIR)/external/ANN/kd_search.cpp \
	$(SBM_MY_DIR)/external/ANN/kd_split.cpp \
	$(SBM_MY_DIR)/external/ANN/kd_tree.cpp \
	$(SBM_MY_DIR)/external/ANN/kd_util.cpp \
	$(SBM_MY_DIR)/external/ANN/perf.cpp \
	$(SBM_MY_DIR)/external/parser/Bchart.cpp \
	$(SBM_MY_DIR)/external/parser/BchartSm.cpp \
	$(SBM_MY_DIR)/external/parser/Bst.cpp \
	$(SBM_MY_DIR)/external/parser/ChartBase.cpp \
	$(SBM_MY_DIR)/external/parser/ClassRule.cpp \
	$(SBM_MY_DIR)/external/parser/CntxArray.cpp \
	$(SBM_MY_DIR)/external/parser/CombineBests.cpp \
	$(SBM_MY_DIR)/external/parser/ECArgs.cpp \
	$(SBM_MY_DIR)/external/parser/Edge.cpp \
	$(SBM_MY_DIR)/external/parser/EdgeHeap.cpp \
	$(SBM_MY_DIR)/external/parser/extraMain.cpp \
	$(SBM_MY_DIR)/external/parser/edgeSubFns.cpp \
	$(SBM_MY_DIR)/external/parser/EgsFromTree.cpp \
	$(SBM_MY_DIR)/external/parser/ewDciTokStrm.cpp \
	$(SBM_MY_DIR)/external/parser/FBinaryArray.cpp \
	$(SBM_MY_DIR)/external/parser/Feat.cpp \
	$(SBM_MY_DIR)/external/parser/Feature.cpp \
	$(SBM_MY_DIR)/external/parser/FeatureTree.cpp \
	$(SBM_MY_DIR)/external/parser/fhSubFns.cpp \
	$(SBM_MY_DIR)/external/parser/Field.cpp \
	$(SBM_MY_DIR)/external/parser/FullHist.cpp \
	$(SBM_MY_DIR)/external/parser/GotIter.cpp \
	$(SBM_MY_DIR)/external/parser/headFinder.cpp \
	$(SBM_MY_DIR)/external/parser/headFinderCh.cpp \
	$(SBM_MY_DIR)/external/parser/InputTree.cpp \
	$(SBM_MY_DIR)/external/parser/Item.cpp \
	$(SBM_MY_DIR)/external/parser/Link.cpp \
	$(SBM_MY_DIR)/external/parser/MeChart.cpp \
	$(SBM_MY_DIR)/external/parser/Params.cpp \
	$(SBM_MY_DIR)/external/parser/ParseStats.cpp \
	$(SBM_MY_DIR)/external/parser/SentRep.cpp \
	$(SBM_MY_DIR)/external/parser/Term.cpp \
	$(SBM_MY_DIR)/external/parser/TimeIt.cpp \
	$(SBM_MY_DIR)/external/parser/UnitRules.cpp \
	$(SBM_MY_DIR)/external/parser/utils.cpp \
	$(SBM_MY_DIR)/external/parser/ValHeap.cpp \
	$(SBM_MY_DIR)/external/parser/SentRep.cpp 
	
	
	
include $(BUILD_STATIC_LIBRARY)

include $(SBM_LOCAL_PATH)/../../ode/jni/Android.mk
include $(SBM_LOCAL_PATH)/../../wsp/jni/Android.mk
include $(SBM_LOCAL_PATH)/../../bonebus/jni/Android.mk
include $(SBM_LOCAL_PATH)/../../steersuite-1.3/jni/Android.mk

SB_LIB_PATH := ../../../lib
LOCAL_PATH := $(SBM_LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE := sbm
#LOCAL_CFLAGS    :=  -gstabs -g -DBUILD_ANDROID -frtti 
LOCAL_CFLAGS    := -O3 -DBUILD_ANDROID -frtti 
LOCAL_C_INCLUDES := $(SBM_LOCAL_PATH)/$(SBM_MY_DIR) \
					$(SBM_LOCAL_PATH)/../../pythonLib/include/python2.6 \
					$(SBM_LOCAL_PATH)/../../boost \
					$(SBM_LOCAL_PATH)/$(SB_LIB_PATH)/boost \
					$(SBM_LOCAL_PATH)/../../include \
					$(SBM_LOCAL_PATH)/$(SB_LIB_PATH)/vhcl/include \
					$(SBM_LOCAL_PATH)/$(SB_LIB_PATH)/bonebus/include \
					$(SBM_LOCAL_PATH)/$(SB_LIB_PATH)/vhmsg/vhmsg-c/include \
					$(SBM_LOCAL_PATH)/$(SB_LIB_PATH)/wsp/wsp/include \
					$(SBM_LOCAL_PATH)/../../../core/smartbody/steersuite-1.3/external/ \
					$(SBM_LOCAL_PATH)/../../../core/smartbody/steersuite-1.3/external/parser/ \
					$(SBM_LOCAL_PATH)/../../../core/smartbody/steersuite-1.3/steerlib/include \
					$(SBM_LOCAL_PATH)/../../../core/smartbody/steersuite-1.3/pprAI/include \
					$(SBM_LOCAL_PATH)/../../../core/smartbody/sbm-debugger/lib \
					$(SBM_LOCAL_PATH)/../../../core/smartbody/ode/include	
LOCAL_SRC_FILES := $(SBM_MY_DIR)/sr/sr_alg.cpp \
	$(SBM_MY_DIR)/sr/sr_array.cpp \
	$(SBM_MY_DIR)/sr/sr_box.cpp \
	$(SBM_MY_DIR)/sr/sr_buffer.cpp \
	$(SBM_MY_DIR)/sr/sr_camera.cpp \
	$(SBM_MY_DIR)/sr/sr_color.cpp \
	$(SBM_MY_DIR)/sr/sr.cpp \
	$(SBM_MY_DIR)/sr/sr_cylinder.cpp \
	$(SBM_MY_DIR)/sr/sr_euler.cpp \
	$(SBM_MY_DIR)/sr/sr_event.cpp \
	$(SBM_MY_DIR)/sr/sr_geo2.cpp \
	$(SBM_MY_DIR)/sr/sr_hash_table.cpp \
	$(SBM_MY_DIR)/sr/sr_input.cpp \
	$(SBM_MY_DIR)/sr/sr_light.cpp \
	$(SBM_MY_DIR)/sr/sr_line.cpp \
	$(SBM_MY_DIR)/sr/sr_lines.cpp \
	$(SBM_MY_DIR)/sr/sr_mat.cpp \
	$(SBM_MY_DIR)/sr/sr_material.cpp \
	$(SBM_MY_DIR)/sr/sr_model.cpp \
	$(SBM_MY_DIR)/sr/sr_model_import_obj.cpp \
	$(SBM_MY_DIR)/sr/sr_model_export_iv.cpp \
	$(SBM_MY_DIR)/sr/sr_output.cpp \
	$(SBM_MY_DIR)/sr/sr_path_array.cpp \
	$(SBM_MY_DIR)/sr/sr_plane.cpp \
	$(SBM_MY_DIR)/sr/sr_sn_colorsurf.cpp \
	$(SBM_MY_DIR)/sr/sr_points.cpp \
	$(SBM_MY_DIR)/sr/sr_polygon.cpp \
	$(SBM_MY_DIR)/sr/sr_polygons.cpp \
	$(SBM_MY_DIR)/sr/sr_quat.cpp \
	$(SBM_MY_DIR)/sr/sr_sa_bbox.cpp \
	$(SBM_MY_DIR)/sr/sr_sa.cpp \
	$(SBM_MY_DIR)/sr/sr_sa_event.cpp \
	$(SBM_MY_DIR)/sr/sr_sa_render_mode.cpp \
	$(SBM_MY_DIR)/sr/sr_shared_ptr.cpp \
	$(SBM_MY_DIR)/sr/sr_sn.cpp \
	$(SBM_MY_DIR)/sr/sr_sn_editor.cpp \
	$(SBM_MY_DIR)/sr/sr_sn_group.cpp \
	$(SBM_MY_DIR)/sr/sr_sn_matrix.cpp \
	$(SBM_MY_DIR)/sr/sr_sn_shape.cpp \
	$(SBM_MY_DIR)/sr/sr_sphere.cpp \
	$(SBM_MY_DIR)/sr/sr_spline.cpp \
	$(SBM_MY_DIR)/sr/sr_string_array.cpp \
	$(SBM_MY_DIR)/sr/sr_string.cpp \
	$(SBM_MY_DIR)/sr/sr_timer.cpp \
	$(SBM_MY_DIR)/sr/sr_trackball.cpp \
	$(SBM_MY_DIR)/sr/sr_tree.cpp \
	$(SBM_MY_DIR)/sr/sr_triangle.cpp \
	$(SBM_MY_DIR)/sr/sr_vec2.cpp \
	$(SBM_MY_DIR)/sr/sr_vec.cpp \
	$(SBM_MY_DIR)/sr/sr_viewer.cpp \
	$(SBM_MY_DIR)/sk/sk_channel.cpp \
	$(SBM_MY_DIR)/sk/sk_channel_array.cpp \
	$(SBM_MY_DIR)/sk/sk_joint.cpp \
	$(SBM_MY_DIR)/sk/sk_joint_euler.cpp \
	$(SBM_MY_DIR)/sk/sk_joint_name.cpp \
	$(SBM_MY_DIR)/sk/sk_joint_pos.cpp \
	$(SBM_MY_DIR)/sk/sk_joint_quat.cpp \
	$(SBM_MY_DIR)/sk/sk_joint_swing_twist.cpp \
	$(SBM_MY_DIR)/sk/sk_motion.cpp \
	$(SBM_MY_DIR)/sk/sk_motion_io.cpp \
	$(SBM_MY_DIR)/sk/sk_posture.cpp \
	$(SBM_MY_DIR)/sk/sk_scene.cpp \
	$(SBM_MY_DIR)/sk/sk_skeleton.cpp \
	$(SBM_MY_DIR)/sk/sk_skeleton_io.cpp \
	$(SBM_MY_DIR)/sk/sk_vec_limits.cpp \
	$(SBM_MY_DIR)/controllers/me_controller_context.cpp \
	$(SBM_MY_DIR)/controllers/me_controller_context_proxy.cpp \
	$(SBM_MY_DIR)/controllers/me_controller.cpp \
	$(SBM_MY_DIR)/controllers/me_controller_tree_root.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_adshr_envelope.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_blend.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_channel_writer.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_container.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_curve_writer.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_interpolator.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_lifecycle_test.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_motion.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_periodic_replay.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_pose.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_scheduler2.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_time_shift_warp.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_unary.cpp \
	$(SBM_MY_DIR)/controllers/me_default_prune_policy.cpp \
	$(SBM_MY_DIR)/sbm/BMLDefs.cpp \
	$(SBM_MY_DIR)/bml/behavior_scheduler_constant_speed.cpp \
	$(SBM_MY_DIR)/bml/behavior_scheduler.cpp \
	$(SBM_MY_DIR)/bml/behavior_scheduler_fixed.cpp \
	$(SBM_MY_DIR)/bml/behavior_span.cpp \
	$(SBM_MY_DIR)/bml/bml_animation.cpp \
	$(SBM_MY_DIR)/bml/bml_bodyreach.cpp \
	$(SBM_MY_DIR)/bml/bml_constraint.cpp \
	$(SBM_MY_DIR)/bml/bml.cpp \
	$(SBM_MY_DIR)/bml/bml_event.cpp \
	$(SBM_MY_DIR)/bml/bml_face.cpp \
	$(SBM_MY_DIR)/bml/bml_gaze.cpp \
	$(SBM_MY_DIR)/bml/bml_general_param.cpp \
	$(SBM_MY_DIR)/bml/bml_gesture.cpp \
	$(SBM_MY_DIR)/bml/bml_grab.cpp \
	$(SBM_MY_DIR)/bml/bml_interrupt.cpp \
	$(SBM_MY_DIR)/bml/bml_locomotion.cpp \
	$(SBM_MY_DIR)/bml/bml_processor.cpp \
	$(SBM_MY_DIR)/bml/bml_quickdraw.cpp \
	$(SBM_MY_DIR)/bml/bml_speech.cpp \
	$(SBM_MY_DIR)/bml/bml_sync_point.cpp \
	$(SBM_MY_DIR)/bml/bml_target.cpp \
	$(SBM_MY_DIR)/bml/bml_saccade.cpp \
	$(SBM_MY_DIR)/bml/bml_states.cpp \
	$(SBM_MY_DIR)/sbm/Event.cpp \
	$(SBM_MY_DIR)/sbm/GenericViewer.cpp \
	$(SBM_MY_DIR)/sbm/gwiz_cmdl.cpp \
	$(SBM_MY_DIR)/sbm/gwiz_math.cpp \
	$(SBM_MY_DIR)/sbm/gwiz_spline.cpp \
	$(SBM_MY_DIR)/sbm/Heightfield.cpp \
	$(SBM_MY_DIR)/sbm/joint_logger.cpp \
	$(SBM_MY_DIR)/sbm/lin_win.cpp \
	$(SBM_MY_DIR)/sbm/locomotion_cmds.cpp \
	$(SBM_MY_DIR)/sbm/mcontrol_callbacks.cpp \
	$(SBM_MY_DIR)/sbm/mcontrol_util.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_basic_locomotion.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_ccd_IK.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_constraint.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_data_driven_reach.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_data_interpolation.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_example_body_reach.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_physics_controller.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_examples.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_eyelid.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_face.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_gaze_alg.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_gaze.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_gaze_joint.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_gaze_keymap.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_gaze_target.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_reach_IK.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_IK.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_IK_scenario.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_jacobian_IK.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_lilt_try.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_limb.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_breathing.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_breathing_interface.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_analysis.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_anim_global_info.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_balance.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_func.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_height_offset.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_IK.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_joint_info.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_limb_anim.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_limb.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_limb_direction_planner.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_navigator.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_pawn.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_quadratic_synchronizer.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_routine.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_simple.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_speed_accelerator.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_terrain.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_locomotion_timing_space.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_motion_example.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_motion_parameter.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_motion_player.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_motion_timewarp.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_navigation_circle.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_navigation_waypoint.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_param_animation.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_param_animation_data.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_param_animation_utilities.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_quick_draw.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_hand.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_motion_profile.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_barycentric_interpolation.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_inverse_interpolation.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_simple_gaze.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_step_turn.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_tether.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_ublas.cpp \
	$(SBM_MY_DIR)/controllers/MeCtBlendEngine.cpp \
	$(SBM_MY_DIR)/sbm/me_utilities.cpp \
	$(SBM_MY_DIR)/sbm/ParserFBX.cpp \
	$(SBM_MY_DIR)/sbm/ParserOpenCOLLADA.cpp \
	$(SBM_MY_DIR)/sbm/remote_speech.cpp \
	$(SBM_MY_DIR)/sbm/local_speech.cpp \
	$(SBM_MY_DIR)/sbm/resource_cmds.cpp \
	$(SBM_MY_DIR)/sbm/Resource.cpp \
	$(SBM_MY_DIR)/sbm/ResourceManager.cpp \
	$(SBM_MY_DIR)/sbm/sbm_audio.cpp \
	$(SBM_MY_DIR)/controllers/MeCtBodyReachState.cpp \
	$(SBM_MY_DIR)/controllers/MeCtReachEngine.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_saccade.cpp \
	$(SBM_MY_DIR)/sbm/sbm_character.cpp \
	$(SBM_MY_DIR)/sbm/sbm_deformable_mesh.cpp \
	$(SBM_MY_DIR)/sbm/sbm_pawn.cpp \
	$(SBM_MY_DIR)/sbm/sbm_speech_audiofile.cpp \
	$(SBM_MY_DIR)/sbm/sbm_speech.cpp \
	$(SBM_MY_DIR)/sbm/sbm_speech_impl_skeleton.cpp \
	$(SBM_MY_DIR)/sbm/sbm_test_cmds.cpp \
	$(SBM_MY_DIR)/sbm/sr_arg_buff.cpp \
	$(SBM_MY_DIR)/sbm/sr_cmd_line.cpp \
	$(SBM_MY_DIR)/sbm/sr_cmd_seq.cpp \
	$(SBM_MY_DIR)/sbm/sr_hash_map.cpp \
	$(SBM_MY_DIR)/sbm/sr_linear_curve.cpp \
	$(SBM_MY_DIR)/sbm/sr_spline_curve.cpp \
	$(SBM_MY_DIR)/sbm/sr_synch_points.cpp \
	$(SBM_MY_DIR)/sbm/text_speech.cpp \
	$(SBM_MY_DIR)/sbm/time_profiler.cpp \
	$(SBM_MY_DIR)/sbm/time_regulator.cpp \
	$(SBM_MY_DIR)/sbm/xercesc_utils.cpp \
	$(SBM_MY_DIR)/sbm/Physics/SbmColObject.cpp \
	$(SBM_MY_DIR)/sbm/Physics/SbmPhysicsSim.cpp \
	$(SBM_MY_DIR)/sbm/Physics/SbmPhysicsSimODE.cpp \
	$(SBM_MY_DIR)/sbm/SteeringAgent.cpp \
	$(SBM_MY_DIR)/sbm/SteerSuiteEnginerDriver.cpp \
	$(SBM_MY_DIR)/sb/SBAttribute.cpp \
	$(SBM_MY_DIR)/sb/SBAttributeManager.cpp \
	$(SBM_MY_DIR)/sb/SBObject.cpp \
	$(SBM_MY_DIR)/sb/SBObserver.cpp \
	$(SBM_MY_DIR)/sb/SBSubject.cpp \
	$(SBM_MY_DIR)/sbm/DefaultAttributeTable.cpp \
	$(SBM_MY_DIR)/sb/SBScene.cpp \
	$(SBM_MY_DIR)/sbm/KinectProcessor.cpp \
	$(SBM_MY_DIR)/controllers/me_ct_data_receiver.cpp \
	$(SBM_MY_DIR)/sb/SBCharacter.cpp \
	$(SBM_MY_DIR)/sb/SBPawn.cpp \
	$(SBM_MY_DIR)/sb/SBJoint.cpp \
	$(SBM_MY_DIR)/sb/SBSkeleton.cpp \
	$(SBM_MY_DIR)/sb/SBController.cpp \
	$(SBM_MY_DIR)/sbm/sr_path_list.cpp \
	$(SBM_MY_DIR)/sb/SBPython.cpp \
	$(SBM_MY_DIR)/sb/SBPython1.cpp \
	$(SBM_MY_DIR)/sb/SBPython2.cpp \
	$(SBM_MY_DIR)/sb/SBPython3.cpp \
	$(SBM_MY_DIR)/sb/SBPython4.cpp \
	$(SBM_MY_DIR)/sb/SBPythonClass.cpp \
	$(SBM_MY_DIR)/sb/SBSimulationManager.cpp \
	$(SBM_MY_DIR)/sb/SBBmlProcessor.cpp \
	$(SBM_MY_DIR)/sb/SBAnimationState.cpp \
	$(SBM_MY_DIR)/sb/SBMotionBlendBase.cpp \
	$(SBM_MY_DIR)/sb/SBAnimationTransition.cpp \
	$(SBM_MY_DIR)/sb/SBAnimationStateManager.cpp \
	$(SBM_MY_DIR)/sb/SBSteerManager.cpp \
	$(SBM_MY_DIR)/sb/SBSteerAgent.cpp \
	$(SBM_MY_DIR)/sb/SBReachManager.cpp \
	$(SBM_MY_DIR)/sb/SBReach.cpp \
	$(SBM_MY_DIR)/sb/SBServiceManager.cpp \
	$(SBM_MY_DIR)/sb/SBService.cpp \
	$(SBM_MY_DIR)/sb/SBMotion.cpp \
	$(SBM_MY_DIR)/sb/SBScript.cpp	\
	$(SBM_MY_DIR)/sb/SBFaceDefinition.cpp \
	$(SBM_MY_DIR)/sb/SBPhysicsManager.cpp \
	$(SBM_MY_DIR)/sb/SBCollisionManager.cpp \
	$(SBM_MY_DIR)/sb/SBJointMapManager.cpp \
	$(SBM_MY_DIR)/sb/SBJointMap.cpp \
	$(SBM_MY_DIR)/sbm/SteerPath.cpp \
	$(SBM_MY_DIR)/sb/SBParser.cpp \
	$(SBM_MY_DIR)/sb/SBParseNode.cpp \
	$(SBM_MY_DIR)/sbm/nvbg.cpp \
	$(SBM_MY_DIR)/sbm/MiniBrain.cpp \
	$(SBM_MY_DIR)/sb/SBBehavior.cpp \
	$(SBM_MY_DIR)/sbm/ParserBVH.cpp \
	$(SBM_MY_DIR)/sbm/ParserASFAMC.cpp \
	$(SBM_MY_DIR)/sb/SBBoneBusManager.cpp \
	$(SBM_MY_DIR)/sb/SBGestureMapManager.cpp \
	$(SBM_MY_DIR)/sb/SBGestureMap.cpp \
	$(SBM_MY_DIR)/sbm/SbmDebuggerServer.cpp \
	$(SBM_MY_DIR)/sbm/SbmDebuggerClient.cpp \
	$(SBM_MY_DIR)/sbm/SbmDebuggerUtility.cpp \
	$(SBM_MY_DIR)/sb/SBPhonemeManager.cpp \
	$(SBM_MY_DIR)/sb/SBPhoneme.cpp \
	$(SBM_MY_DIR)/sb/SBBehaviorSet.cpp \
	$(SBM_MY_DIR)/sb/SBBehaviorSetManager.cpp \
	$(SBM_MY_DIR)/sbm/action_unit.cpp
	
	
LOCAL_LDLIBS    := -llog 
#LOCAL_LDLIBS    := -llog -gstabs	  
#LOCAL_STATIC_LIBRARIES := xerces-prebuilt boost-filesystem-prebuilt boost-system-prebuilt boost-regex-prebuilt lapack blas f2c vhcl wsp vhmsg bonebus iconv-prebuilt pprAI steerlib ann ode 
LOCAL_STATIC_LIBRARIES := xerces-prebuilt boost-filesystem-prebuilt boost-system-prebuilt boost-regex-prebuilt python-prebuilt boost-python-prebuilt lapack blas f2c vhcl wsp vhmsg bonebus iconv-prebuilt pprAI steerlib ann ode  
#LOCAL_SHARED_LIBRARIES := python-prebuilt 
include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)







