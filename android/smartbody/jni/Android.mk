LOCAL_SHORT_COMMANDS := true

USE_CEREVOICE:=true


SB_LOCAL_PATH := $(call my-dir)
LOCAL_PATH := $(SB_LOCAL_PATH)
SB_MY_DIR := ../../../core/smartbody/SmartBody/src
ANDROID_LIB_DIR := ../../lib
BOOST_LIB_DIR := ../../boost_1_59/lib
BOOST_INCLUDE_DIR := ../../boost_1_59/include
ANDROID_DIR := ../../
CEREVOICE_LIB_DIR := ../../cerevoice_lib
LIB_DIR := ../../../lib


ifeq ($(USE_CEREVOICE),true)

include $(CLEAR_VARS)
LOCAL_MODULE := cerevoice-eng
LOCAL_SRC_FILES := $(CEREVOICE_LIB_DIR)/libs/libcerevoice_eng.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := cerevoice-pmod
LOCAL_SRC_FILES := $(CEREVOICE_LIB_DIR)/libs/libcerevoice_pmod.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := cerevoice
LOCAL_SRC_FILES := $(CEREVOICE_LIB_DIR)/libs/libcerevoice.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := cerehts
LOCAL_SRC_FILES := $(CEREVOICE_LIB_DIR)/libs/libcerehts.a
include $(PREBUILT_STATIC_LIBRARY)

CEREVOICE_LIBS:= cerevoice-eng cerevoice-pmod cerehts cerevoice

else
CEREVOICE_LIBS:=
endif

include $(CLEAR_VARS)
LOCAL_MODULE := xerces-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libxerces-c.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := boost-filesystem-prebuilt
LOCAL_SRC_FILES := $(BOOST_LIB_DIR)/libboost_filesystem-gcc-mt-1_59.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := boost-system-prebuilt
LOCAL_SRC_FILES := $(BOOST_LIB_DIR)/libboost_system-gcc-mt-1_59.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := boost-regex-prebuilt
LOCAL_SRC_FILES := $(BOOST_LIB_DIR)/libboost_regex-gcc-mt-1_59.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := python-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libpython2.7.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := python-prebuilt-share
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libpython2.7.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := boost-python-prebuilt
LOCAL_SRC_FILES := $(BOOST_LIB_DIR)/libboost_python-gcc-mt-1_59.a
LOCAL_SHARED_LIBRARIES := python-prebuilt-share	
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

include $(CLEAR_VARS)
LOCAL_MODULE := estbase-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libestbase.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := estools-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libestools.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := eststring-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libeststring.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := festival-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libFestival.a
LOCAL_STATIC_LIBRARIES := estbase-prebuilt estools-prebuilt eststring-prebuilt
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := openal
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libopenal.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := alut
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libopenalut.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := tremolo
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libtremolo.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := sndfile
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libsndfile.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := proto-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libprotobuf.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)	
include $(CLEAR_VARS)
LOCAL_MODULE := ann
LOCAL_CFLAGS    := -DBUILD_ANDROID -frtti -fexceptions 
LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(ANDROID_DIR)/gl-wes-v2/src \
					$(SB_LOCAL_PATH)/$(SB_MY_DIR) \
					

LOCAL_SRC_FILES := $(SB_MY_DIR)/external/parser/Bchart.cpp \
	$(SB_MY_DIR)/external/parser/BchartSm.cpp \
	$(SB_MY_DIR)/external/parser/Bst.cpp \
	$(SB_MY_DIR)/external/parser/ChartBase.cpp \
	$(SB_MY_DIR)/external/parser/ClassRule.cpp \
	$(SB_MY_DIR)/external/parser/CntxArray.cpp \
	$(SB_MY_DIR)/external/parser/CombineBests.cpp \
	$(SB_MY_DIR)/external/parser/ECArgs.cpp \
	$(SB_MY_DIR)/external/parser/Edge.cpp \
	$(SB_MY_DIR)/external/parser/EdgeHeap.cpp \
	$(SB_MY_DIR)/external/parser/extraMain.cpp \
	$(SB_MY_DIR)/external/parser/edgeSubFns.cpp \
	$(SB_MY_DIR)/external/parser/EgsFromTree.cpp \
	$(SB_MY_DIR)/external/parser/ewDciTokStrm.cpp \
	$(SB_MY_DIR)/external/parser/FBinaryArray.cpp \
	$(SB_MY_DIR)/external/parser/Feat.cpp \
	$(SB_MY_DIR)/external/parser/Feature.cpp \
	$(SB_MY_DIR)/external/parser/FeatureTree.cpp \
	$(SB_MY_DIR)/external/parser/fhSubFns.cpp \
	$(SB_MY_DIR)/external/parser/Field.cpp \
	$(SB_MY_DIR)/external/parser/FullHist.cpp \
	$(SB_MY_DIR)/external/parser/GotIter.cpp \
	$(SB_MY_DIR)/external/parser/headFinder.cpp \
	$(SB_MY_DIR)/external/parser/headFinderCh.cpp \
	$(SB_MY_DIR)/external/parser/InputTree.cpp \
	$(SB_MY_DIR)/external/parser/Item.cpp \
	$(SB_MY_DIR)/external/parser/Link.cpp \
	$(SB_MY_DIR)/external/parser/MeChart.cpp \
	$(SB_MY_DIR)/external/parser/Params.cpp \
	$(SB_MY_DIR)/external/parser/ParseStats.cpp \
	$(SB_MY_DIR)/external/parser/SentRep.cpp \
	$(SB_MY_DIR)/external/parser/Term.cpp \
	$(SB_MY_DIR)/external/parser/TimeIt.cpp \
	$(SB_MY_DIR)/external/parser/UnitRules.cpp \
	$(SB_MY_DIR)/external/parser/utils.cpp \
	$(SB_MY_DIR)/external/parser/ValHeap.cpp \
	$(SB_MY_DIR)/external/perlin/perlin.cpp \
	$(SB_MY_DIR)/external/recast/Recast.cpp \
	$(SB_MY_DIR)/external/recast/RecastAlloc.cpp \
	$(SB_MY_DIR)/external/recast/RecastArea.cpp \
	$(SB_MY_DIR)/external/recast/RecastContour.cpp \
	$(SB_MY_DIR)/external/recast/RecastFilter.cpp \
	$(SB_MY_DIR)/external/recast/RecastLayers.cpp \
	$(SB_MY_DIR)/external/recast/RecastMesh.cpp \
	$(SB_MY_DIR)/external/recast/RecastMeshDetail.cpp \
	$(SB_MY_DIR)/external/recast/RecastRasterization.cpp \
	$(SB_MY_DIR)/external/recast/RecastRegion.cpp \
	$(SB_MY_DIR)/external/recast/DetourAlloc.cpp \
	$(SB_MY_DIR)/external/recast/DetourCommon.cpp \
	$(SB_MY_DIR)/external/recast/DetourNavMesh.cpp \
	$(SB_MY_DIR)/external/recast/DetourNavMeshBuilder.cpp \
	$(SB_MY_DIR)/external/recast/DetourNavMeshQuery.cpp \
	$(SB_MY_DIR)/external/recast/DetourNode.cpp \
	$(SB_MY_DIR)/external/SOIL/image_DXT.c \
	$(SB_MY_DIR)/external/SOIL/image_helper.c \
	$(SB_MY_DIR)/external/SOIL/SOIL.c \
	$(SB_MY_DIR)/external/SOIL/stb_image_aug.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/adler32.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/compress.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/crc32.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/deflate.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/example.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/gzclose.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/gzlib.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/gzread.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/gzwrite.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/infback.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/inffast.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/inflate.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/inftrees.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/ioapi.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/trees.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/uncompr.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/unzip.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/zip.c \
	$(SB_MY_DIR)/external/zlib-1.2.5/zutil.c \
	$(SB_MY_DIR)/external/rply/rply.c \
	
include $(BUILD_STATIC_LIBRARY)	

include $(SB_LOCAL_PATH)/../../ode/jni/Android.mk
include $(SB_LOCAL_PATH)/../../vhmsg/jni/Android.mk
include $(SB_LOCAL_PATH)/../../bonebus/jni/Android.mk
include $(SB_LOCAL_PATH)/../../steersuite-1.3/jni/Android.mk

SB_LIB_PATH := ../../../lib
LOCAL_PATH := $(SB_LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE := smartbody
LOCAL_CFLAGS    := -O3 -DBUILD_ANDROID -frtti -fexceptions -g -DASSIMP_BUILD_NO_OWN_ZLIB
LOCAL_C_INCLUDES := $(SB_LOCAL_PATH)/$(SB_MY_DIR) \
					$(SB_LOCAL_PATH)/../../pythonLib_27/include/python2.7 \
					$(SB_LOCAL_PATH)/../../boost \
					$(SB_LOCAL_PATH)/$(BOOST_INCLUDE_DIR) \
					$(SB_LOCAL_PATH)/../../include \
					$(SB_LOCAL_PATH)/../../include/eigen \
					$(SB_LOCAL_PATH)/$(CEREVOICE_LIB_DIR)/cerevoice_eng/include \
					$(SB_LOCAL_PATH)/$(SB_LIB_PATH)/festival/speech_tools/include \
					$(SB_LOCAL_PATH)/$(SB_LIB_PATH)/festival/festival/src/include \
					$(SB_LOCAL_PATH)/$(SB_LIB_PATH)/festival/festival/src/modules/VHDuration \
					$(SB_LOCAL_PATH)/$(SB_LIB_PATH)/vhcl/include \
					$(SB_LOCAL_PATH)/$(SB_LIB_PATH)/bonebus/include \
					$(SB_LOCAL_PATH)/$(SB_LIB_PATH)/vhmsg/vhmsg-c/include \
					$(SB_LOCAL_PATH)/$(SB_LIB_PATH)/boostnumeric \
					$(SB_LOCAL_PATH)/../../../core/smartbody/SmartBody/src/external/protobuf/include \
					$(SB_LOCAL_PATH)/../../../core/smartbody/steersuite-1.3/external/ \
					$(SB_LOCAL_PATH)/../../../core/smartbody/steersuite-1.3/external/parser/ \
					$(SB_LOCAL_PATH)/../../../core/smartbody/steersuite-1.3/steerlib/include \
					$(SB_LOCAL_PATH)/../../../core/smartbody/steersuite-1.3/pprAI/include \
					$(SB_LOCAL_PATH)/../../../core/smartbody/sbm-debugger/lib \
					$(SB_LOCAL_PATH)/../../../core/smartbody/ode/include \
					
					
LOCAL_SRC_FILES := $(SB_MY_DIR)/sr/sr_alg.cpp \
	$(SB_MY_DIR)/sr/sr_array.cpp \
	$(SB_MY_DIR)/sr/sr_box.cpp \
	$(SB_MY_DIR)/sr/sr_buffer.cpp \
	$(SB_MY_DIR)/sr/sr_camera.cpp \
	$(SB_MY_DIR)/sr/sr_color.cpp \
	$(SB_MY_DIR)/sr/sr.cpp \
	$(SB_MY_DIR)/sr/sr_cylinder.cpp \
	$(SB_MY_DIR)/sr/sr_euler.cpp \
	$(SB_MY_DIR)/sr/sr_event.cpp \
	$(SB_MY_DIR)/sr/sr_geo2.cpp \
	$(SB_MY_DIR)/sr/sr_hash_table.cpp \
	$(SB_MY_DIR)/sr/sr_input.cpp \
	$(SB_MY_DIR)/sr/sr_light.cpp \
	$(SB_MY_DIR)/sr/sr_line.cpp \
	$(SB_MY_DIR)/sr/sr_lines.cpp \
	$(SB_MY_DIR)/sr/sr_mat.cpp \
	$(SB_MY_DIR)/sr/sr_material.cpp \
	$(SB_MY_DIR)/sr/sr_model.cpp \
	$(SB_MY_DIR)/sr/sr_model_import_obj.cpp \
	$(SB_MY_DIR)/sr/sr_model_export_iv.cpp \
	$(SB_MY_DIR)/sr/sr_model_import_ply.cpp \
	$(SB_MY_DIR)/sr/sr_output.cpp \
	$(SB_MY_DIR)/sr/sr_path_array.cpp \
	$(SB_MY_DIR)/sr/sr_plane.cpp \
	$(SB_MY_DIR)/sr/sr_sn_colorsurf.cpp \
	$(SB_MY_DIR)/sr/sr_points.cpp \
	$(SB_MY_DIR)/sr/sr_polygon.cpp \
	$(SB_MY_DIR)/sr/sr_polygons.cpp \
	$(SB_MY_DIR)/sr/sr_quat.cpp \
	$(SB_MY_DIR)/sr/sr_random.cpp \
	$(SB_MY_DIR)/sr/sr_sa_bbox.cpp \
	$(SB_MY_DIR)/sr/sr_sa.cpp \
	$(SB_MY_DIR)/sr/sr_sa_event.cpp \
	$(SB_MY_DIR)/sr/sr_sa_render_mode.cpp \
	$(SB_MY_DIR)/sr/sr_shared_ptr.cpp \
	$(SB_MY_DIR)/sr/sr_sn.cpp \
	$(SB_MY_DIR)/sr/sr_sn_editor.cpp \
	$(SB_MY_DIR)/sr/sr_sn_group.cpp \
	$(SB_MY_DIR)/sr/sr_sn_matrix.cpp \
	$(SB_MY_DIR)/sr/sr_sn_shape.cpp \
	$(SB_MY_DIR)/sr/sr_sphere.cpp \
	$(SB_MY_DIR)/sr/sr_spline.cpp \
	$(SB_MY_DIR)/sr/sr_string_array.cpp \
	$(SB_MY_DIR)/sr/sr_string.cpp \
	$(SB_MY_DIR)/sr/sr_timer.cpp \
	$(SB_MY_DIR)/sr/sr_trackball.cpp \
	$(SB_MY_DIR)/sr/sr_tree.cpp \
	$(SB_MY_DIR)/sr/sr_triangle.cpp \
	$(SB_MY_DIR)/sr/sr_vec2.cpp \
	$(SB_MY_DIR)/sr/sr_vec.cpp \
	$(SB_MY_DIR)/sr/sr_gl.cpp \
	$(SB_MY_DIR)/sr/sr_gl_render_funcs.cpp \
	$(SB_MY_DIR)/sr/sr_sa_gl_render.cpp \
	$(SB_MY_DIR)/sr/sr_viewer.cpp \
	$(SB_MY_DIR)/sk/sk_channel.cpp \
	$(SB_MY_DIR)/sk/sk_channel_array.cpp \
	$(SB_MY_DIR)/sk/sk_joint.cpp \
	$(SB_MY_DIR)/sk/sk_joint_euler.cpp \
	$(SB_MY_DIR)/sk/sk_joint_name.cpp \
	$(SB_MY_DIR)/sk/sk_joint_pos.cpp \
	$(SB_MY_DIR)/sk/sk_joint_quat.cpp \
	$(SB_MY_DIR)/sk/sk_joint_swing_twist.cpp \
	$(SB_MY_DIR)/sk/sk_motion.cpp \
	$(SB_MY_DIR)/sk/sk_motion_io.cpp \
	$(SB_MY_DIR)/sk/sk_posture.cpp \
	$(SB_MY_DIR)/sk/sk_scene.cpp \
	$(SB_MY_DIR)/sk/sk_skeleton.cpp \
	$(SB_MY_DIR)/sk/sk_skeleton_io.cpp \
	$(SB_MY_DIR)/sk/sk_vec_limits.cpp \
	$(SB_MY_DIR)/controllers/me_controller_context.cpp \
	$(SB_MY_DIR)/controllers/me_controller_context_proxy.cpp \
	$(SB_MY_DIR)/controllers/me_controller.cpp \
	$(SB_MY_DIR)/controllers/me_controller_tree_root.cpp \
	$(SB_MY_DIR)/controllers/me_ct_blend.cpp \
	$(SB_MY_DIR)/controllers/me_ct_channel_writer.cpp \
	$(SB_MY_DIR)/controllers/me_ct_container.cpp \
	$(SB_MY_DIR)/controllers/me_ct_curve_writer.cpp \
	$(SB_MY_DIR)/controllers/me_ct_interpolator.cpp \
	$(SB_MY_DIR)/controllers/me_ct_motion.cpp \
	$(SB_MY_DIR)/controllers/me_ct_periodic_replay.cpp \
	$(SB_MY_DIR)/controllers/me_ct_pose.cpp \
	$(SB_MY_DIR)/controllers/me_ct_scheduler2.cpp \
	$(SB_MY_DIR)/controllers/me_ct_time_shift_warp.cpp \
	$(SB_MY_DIR)/controllers/me_ct_unary.cpp \
	$(SB_MY_DIR)/controllers/me_default_prune_policy.cpp \
	$(SB_MY_DIR)/sbm/BMLDefs.cpp \
	$(SB_MY_DIR)/bml/behavior_scheduler_constant_speed.cpp \
	$(SB_MY_DIR)/bml/behavior_scheduler.cpp \
	$(SB_MY_DIR)/bml/behavior_scheduler_gesture.cpp \
	$(SB_MY_DIR)/bml/behavior_scheduler_fixed.cpp \
	$(SB_MY_DIR)/bml/behavior_span.cpp \
	$(SB_MY_DIR)/bml/bml_animation.cpp \
	$(SB_MY_DIR)/bml/bml_bodyreach.cpp \
	$(SB_MY_DIR)/bml/bml_constraint.cpp \
	$(SB_MY_DIR)/bml/bml.cpp \
	$(SB_MY_DIR)/bml/bml_event.cpp \
	$(SB_MY_DIR)/bml/bml_face.cpp \
	$(SB_MY_DIR)/bml/bml_gaze.cpp \
	$(SB_MY_DIR)/bml/bml_general_param.cpp \
	$(SB_MY_DIR)/bml/bml_gesture.cpp \
	$(SB_MY_DIR)/bml/bml_grab.cpp \
	$(SB_MY_DIR)/bml/bml_interrupt.cpp \
	$(SB_MY_DIR)/bml/bml_locomotion.cpp \
	$(SB_MY_DIR)/bml/bml_processor.cpp \
	$(SB_MY_DIR)/bml/bml_speech.cpp \
	$(SB_MY_DIR)/bml/bml_sync_point.cpp \
	$(SB_MY_DIR)/bml/bml_target.cpp \
	$(SB_MY_DIR)/bml/bml_saccade.cpp \
	$(SB_MY_DIR)/bml/bml_states.cpp \
	$(SB_MY_DIR)/bml/bml_noise.cpp \
	$(SB_MY_DIR)/bml/BMLAnimationObject.cpp \
	$(SB_MY_DIR)/bml/BMLBodyObject.cpp \
	$(SB_MY_DIR)/bml/BMLConstraintObject.cpp \
	$(SB_MY_DIR)/bml/BMLEventObject.cpp \
	$(SB_MY_DIR)/bml/BMLFaceObject.cpp \
	$(SB_MY_DIR)/bml/BMLGazeObject.cpp \
	$(SB_MY_DIR)/bml/BMLGestureObject.cpp \
	$(SB_MY_DIR)/bml/BMLHeadObject.cpp \
	$(SB_MY_DIR)/bml/BMLLocomotionObject.cpp \
	$(SB_MY_DIR)/bml/BMLObject.cpp \
	$(SB_MY_DIR)/bml/BMLReachObject.cpp \
	$(SB_MY_DIR)/bml/BMLSpeechObject.cpp \
	$(SB_MY_DIR)/bml/BMLSaccadeObject.cpp \
	$(SB_MY_DIR)/bml/BMLHandObject.cpp \
	$(SB_MY_DIR)/bml/BMLStateObject.cpp \
	$(SB_MY_DIR)/bml/BMLNoiseObject.cpp	 \
	$(SB_MY_DIR)/sbm/GenericViewer.cpp \
	$(SB_MY_DIR)/sbm/gwiz_cmdl.cpp \
	$(SB_MY_DIR)/sbm/gwiz_math.cpp \
	$(SB_MY_DIR)/sbm/gwiz_spline.cpp \
	$(SB_MY_DIR)/sbm/Heightfield.cpp \
	$(SB_MY_DIR)/sbm/lin_win.cpp \
	$(SB_MY_DIR)/sbm/mcontrol_callbacks.cpp \
	$(SB_MY_DIR)/controllers/me_ct_basic_locomotion.cpp \
	$(SB_MY_DIR)/controllers/me_ct_ccd_IK.cpp \
	$(SB_MY_DIR)/controllers/me_ct_constraint.cpp \
	$(SB_MY_DIR)/controllers/me_ct_data_driven_reach.cpp \
	$(SB_MY_DIR)/controllers/me_ct_data_interpolation.cpp \
	$(SB_MY_DIR)/controllers/me_ct_example_body_reach.cpp \
	$(SB_MY_DIR)/controllers/me_ct_physics_controller.cpp \
	$(SB_MY_DIR)/controllers/me_ct_examples.cpp \
	$(SB_MY_DIR)/controllers/me_ct_eyelid.cpp \
	$(SB_MY_DIR)/controllers/me_ct_face.cpp \
	$(SB_MY_DIR)/controllers/me_ct_gaze_alg.cpp \
	$(SB_MY_DIR)/controllers/me_ct_gaze.cpp \
	$(SB_MY_DIR)/controllers/me_ct_gaze_joint.cpp \
	$(SB_MY_DIR)/controllers/me_ct_gaze_keymap.cpp \
	$(SB_MY_DIR)/controllers/me_ct_gaze_target.cpp \
	$(SB_MY_DIR)/controllers/me_ct_reach_IK.cpp \
	$(SB_MY_DIR)/controllers/me_ct_IK.cpp \
	$(SB_MY_DIR)/controllers/me_ct_IK_scenario.cpp \
	$(SB_MY_DIR)/controllers/me_ct_jacobian_IK.cpp \
	$(SB_MY_DIR)/controllers/me_ct_limb.cpp \
	$(SB_MY_DIR)/controllers/me_ct_breathing.cpp \
	$(SB_MY_DIR)/controllers/me_ct_breathing_interface.cpp \
	$(SB_MY_DIR)/controllers/me_ct_locomotion_func.cpp \
	$(SB_MY_DIR)/controllers/me_ct_motion_example.cpp \
	$(SB_MY_DIR)/controllers/me_ct_motion_parameter.cpp \
	$(SB_MY_DIR)/controllers/me_ct_motion_player.cpp \
	$(SB_MY_DIR)/controllers/me_ct_motion_timewarp.cpp \
	$(SB_MY_DIR)/controllers/me_ct_new_locomotion.cpp \
	$(SB_MY_DIR)/controllers/me_ct_param_animation.cpp \
	$(SB_MY_DIR)/controllers/me_ct_param_animation_utilities.cpp \
	$(SB_MY_DIR)/controllers/me_ct_hand.cpp \
	$(SB_MY_DIR)/controllers/me_ct_motion_profile.cpp \
	$(SB_MY_DIR)/controllers/me_ct_barycentric_interpolation.cpp \
	$(SB_MY_DIR)/controllers/me_ct_inverse_interpolation.cpp \
	$(SB_MY_DIR)/controllers/me_ct_simple_gaze.cpp \
	$(SB_MY_DIR)/controllers/me_ct_tether.cpp \
	$(SB_MY_DIR)/controllers/me_ct_ublas.cpp \
	$(SB_MY_DIR)/controllers/me_ct_noise_controller.cpp \
	$(SB_MY_DIR)/controllers/me_ct_motion_recorder.cpp \
	$(SB_MY_DIR)/controllers/me_ct_pose_postprocessing.cpp \
	$(SB_MY_DIR)/controllers/me_ct_motion_graph.cpp \
	$(SB_MY_DIR)/controllers/me_ct_generic_hand.cpp \
	$(SB_MY_DIR)/controllers/RealTimeLipSyncController.cpp \
	$(SB_MY_DIR)/controllers/MeCtBlendEngine.cpp \
	$(SB_MY_DIR)/controllers/MotionAnalysis.cpp \
	$(SB_MY_DIR)/sbm/ParserFBX.cpp \
	$(SB_MY_DIR)/sbm/ParserCOLLADAFast.cpp \
	$(SB_MY_DIR)/sbm/ParserOpenCOLLADA.cpp \
	$(SB_MY_DIR)/sbm/ParserOgre.cpp \
	$(SB_MY_DIR)/sbm/remote_speech.cpp \
	$(SB_MY_DIR)/sbm/local_speech.cpp \
	$(SB_MY_DIR)/sbm/sbm_audio.cpp \
	$(SB_MY_DIR)/controllers/MeCtBodyReachState.cpp \
	$(SB_MY_DIR)/controllers/MeCtReachEngine.cpp \
	$(SB_MY_DIR)/controllers/me_ct_saccade.cpp \
	$(SB_MY_DIR)/sbm/sbm_deformable_mesh.cpp \
	$(SB_MY_DIR)/sbm/sbm_speech_audiofile.cpp \
	$(SB_MY_DIR)/sbm/sbm_speech.cpp \
	$(SB_MY_DIR)/sbm/sbm_speech_impl_skeleton.cpp \
	$(SB_MY_DIR)/sbm/sbm_test_cmds.cpp \
	$(SB_MY_DIR)/sbm/sr_arg_buff.cpp \
	$(SB_MY_DIR)/sbm/sr_cmd_line.cpp \
	$(SB_MY_DIR)/sbm/sr_cmd_seq.cpp \
	$(SB_MY_DIR)/sbm/sr_hash_map.cpp \
	$(SB_MY_DIR)/sbm/sr_linear_curve.cpp \
	$(SB_MY_DIR)/sbm/sr_spline_curve.cpp \
	$(SB_MY_DIR)/sbm/sr_synch_points.cpp \
	$(SB_MY_DIR)/sbm/text_speech.cpp \
	$(SB_MY_DIR)/sbm/time_profiler.cpp \
	$(SB_MY_DIR)/sbm/time_regulator.cpp \
	$(SB_MY_DIR)/sbm/xercesc_utils.cpp \
	$(SB_MY_DIR)/sbm/ODEPhysicsSim.cpp \
	$(SB_MY_DIR)/sbm/PPRAISteeringAgent.cpp \
	$(SB_MY_DIR)/sbm/SteerSuiteEnginerDriver.cpp \
	$(SB_MY_DIR)/sbm/GPU/SbmTexture.cpp \
	$(SB_MY_DIR)/sbm/GPU/SbmShader.cpp \
	$(SB_MY_DIR)/sbm/GPU/SbmBlendFace.cpp \
	$(SB_MY_DIR)/sbm/GPU/VBOData.cpp \
	$(SB_MY_DIR)/sbm/GPU/TBOData.cpp \
	$(SB_MY_DIR)/sbm/GPU/SbmDeformableMeshGPU.cpp \
	$(SB_MY_DIR)/sb/sbm_character.cpp \
	$(SB_MY_DIR)/sb/sbm_pawn.cpp \
	$(SB_MY_DIR)/sb/SBAttribute.cpp \
	$(SB_MY_DIR)/sb/SBAttributeManager.cpp \
	$(SB_MY_DIR)/sb/SBObject.cpp \
	$(SB_MY_DIR)/sb/SBObserver.cpp \
	$(SB_MY_DIR)/sb/SBSubject.cpp \
	$(SB_MY_DIR)/sb/DefaultAttributeTable.cpp \
	$(SB_MY_DIR)/sb/PABlend.cpp \
	$(SB_MY_DIR)/sb/SBScene.cpp \
	$(SB_MY_DIR)/sbm/KinectProcessor.cpp \
	$(SB_MY_DIR)/controllers/me_ct_data_receiver.cpp \
	$(SB_MY_DIR)/sb/SBCharacter.cpp \
	$(SB_MY_DIR)/sb/SBPawn.cpp \
	$(SB_MY_DIR)/sb/SBJoint.cpp \
	$(SB_MY_DIR)/sb/SBSkeleton.cpp \
	$(SB_MY_DIR)/sb/SBController.cpp \
	$(SB_MY_DIR)/sbm/sr_path_list.cpp \
	$(SB_MY_DIR)/sb/SBPython.cpp \
	$(SB_MY_DIR)/sb/SBPythonAnimation.cpp \
	$(SB_MY_DIR)/sb/SBPythonAttribute.cpp \
	$(SB_MY_DIR)/sb/SBPythonCharacter.cpp \
	$(SB_MY_DIR)/sb/SBPythonMath.cpp \
	$(SB_MY_DIR)/sb/SBPythonMesh.cpp \
	$(SB_MY_DIR)/sb/SBPythonMotion.cpp \
	$(SB_MY_DIR)/sb/SBPythonScene.cpp \
	$(SB_MY_DIR)/sb/SBPythonSimulation.cpp \
	$(SB_MY_DIR)/sb/SBPythonSkeleton.cpp \
	$(SB_MY_DIR)/sb/SBPythonSystem.cpp \
	$(SB_MY_DIR)/sb/SBPythonClass.cpp \
	$(SB_MY_DIR)/sb/SBSimulationManager.cpp \
	$(SB_MY_DIR)/sb/SBBmlProcessor.cpp \
	$(SB_MY_DIR)/sb/SBAnimationState.cpp \
	$(SB_MY_DIR)/sb/SBMotionBlendBase.cpp \
	$(SB_MY_DIR)/sb/SBAnimationTransition.cpp \
	$(SB_MY_DIR)/sb/SBAnimationTransitionRule.cpp \
	$(SB_MY_DIR)/sb/SBAnimationStateManager.cpp \
	$(SB_MY_DIR)/sb/SBSteerManager.cpp \
	$(SB_MY_DIR)/sb/SBSteerAgent.cpp \
	$(SB_MY_DIR)/sb/SBReachManager.cpp \
	$(SB_MY_DIR)/sb/SBReach.cpp \
	$(SB_MY_DIR)/sb/SBServiceManager.cpp \
	$(SB_MY_DIR)/sb/SBService.cpp \
	$(SB_MY_DIR)/sb/SBMotion.cpp \
	$(SB_MY_DIR)/sb/SBScript.cpp	\
	$(SB_MY_DIR)/sb/SBFaceDefinition.cpp \
	$(SB_MY_DIR)/sb/SBFaceShiftManager.cpp \
	$(SB_MY_DIR)/sb/SBRealtimeManager.cpp \
	$(SB_MY_DIR)/sb/SBPhysicsManager.cpp \
	$(SB_MY_DIR)/sb/SBCollisionManager.cpp \
	$(SB_MY_DIR)/sb/SBJointMapManager.cpp \
	$(SB_MY_DIR)/sb/SBJointMap.cpp \
	$(SB_MY_DIR)/sbm/SteerPath.cpp \
	$(SB_MY_DIR)/sb/SBParser.cpp \
	$(SB_MY_DIR)/sb/SBParseNode.cpp \
	$(SB_MY_DIR)/sb/nvbg.cpp \
	$(SB_MY_DIR)/sb/MiniBrain.cpp \
	$(SB_MY_DIR)/sb/SBBehavior.cpp \
	$(SB_MY_DIR)/sbm/ParserBVH.cpp \
	$(SB_MY_DIR)/sbm/ParserASFAMC.cpp \
	$(SB_MY_DIR)/sb/SBBoneBusManager.cpp \
	$(SB_MY_DIR)/sb/SBGestureMapManager.cpp \
	$(SB_MY_DIR)/sb/SBGestureMap.cpp \
	$(SB_MY_DIR)/sb/SBDebuggerServer.cpp \
	$(SB_MY_DIR)/sb/SBDebuggerClient.cpp \
	$(SB_MY_DIR)/sb/SBDebuggerUtility.cpp \
	$(SB_MY_DIR)/sb/SBPhonemeManager.cpp \
	$(SB_MY_DIR)/sb/SBPhoneme.cpp \
	$(SB_MY_DIR)/sb/SBBehaviorSet.cpp \
	$(SB_MY_DIR)/sb/SBBehaviorSetManager.cpp \
	$(SB_MY_DIR)/sb/SBNavigationMesh.cpp \
	$(SB_MY_DIR)/sbm/action_unit.cpp \
	$(SB_MY_DIR)/sbm/MiscCommands.cpp \
	$(SB_MY_DIR)/sb/SBColObject.cpp \
	$(SB_MY_DIR)/sb/SBPhysicsSim.cpp \
	$(SB_MY_DIR)/sb/SBEvent.cpp \
	$(SB_MY_DIR)/sb/SBRetarget.cpp \
	$(SB_MY_DIR)/sb/SBRetargetManager.cpp \
	$(SB_MY_DIR)/sb/SBAsset.cpp \
	$(SB_MY_DIR)/sb/SBAssetManager.cpp \
	$(SB_MY_DIR)/sb/SBSpeechManager.cpp \
	$(SB_MY_DIR)/sb/SBVHMsgManager.cpp \
	$(SB_MY_DIR)/sb/SBCommandManager.cpp \
	$(SB_MY_DIR)/sb/SBNavigationMeshManager.cpp \
	$(SB_MY_DIR)/sb/SBAssetHandler.cpp \
    $(SB_MY_DIR)/sb/SBAssetHandlerCOLLADA.cpp \
    $(SB_MY_DIR)/sb/SBAssetHandlerSk.cpp \
    $(SB_MY_DIR)/sb/SBAssetHandlerSkm.cpp \
    $(SB_MY_DIR)/sb/SBAssetHandlerAmc.cpp \
    $(SB_MY_DIR)/sb/SBAssetHandlerAsf.cpp \
    $(SB_MY_DIR)/sb/SBAssetHandlerOgre.cpp \
    $(SB_MY_DIR)/sb/SBAssetHandlerObj.cpp \
    $(SB_MY_DIR)/sb/SBAssetHandlerBvh.cpp \
	$(SB_MY_DIR)/sb/SBAssetHandlerPly.cpp \
	$(SB_MY_DIR)/sb/SBAssetHandlerHdr.cpp \
    $(SB_MY_DIR)/sb/SBAssetHandlerSBMeshBinary.cpp \
    $(SB_MY_DIR)/sb/SBAssetHandlerSkmb.cpp \
	$(SB_MY_DIR)/sb/SBMotionGraph.cpp \
	$(SB_MY_DIR)/sb/SBHandConfiguration.cpp \
	$(SB_MY_DIR)/sb/SBHandConfigurationManager.cpp \
	$(SB_MY_DIR)/sb/SBHandSynthesis.cpp \
	$(SB_MY_DIR)/sb/SBRigNode.cpp \
	$(SB_MY_DIR)/sb/SBUtilities.cpp \
    $(SB_MY_DIR)/sb/smartbody-c-dll.cpp \
	$(SB_MY_DIR)/protocols/sbmesh.pb.cpp \
	$(SB_MY_DIR)/protocols/sbmotion.pb.cpp \
	$(SB_MY_DIR)/protocols/sbutilities.pb.cpp

LOCAL_LDLIBS    := -landroid -llog -lGLESv3

#LOCAL_STATIC_LIBRARIES := ann gl-wes xerces-prebuilt boost-filesystem-prebuilt boost-system-prebuilt boost-regex-prebuilt boost-python-prebuilt lapack blas f2c vhcl vhmsg bonebus iconv-prebuilt pprAI steerlib ode  openal alut tremolo sndfile python-prebuilt proto-prebuilt $(CEREVOICE_LIBS)
LOCAL_STATIC_LIBRARIES := ann xerces-prebuilt boost-filesystem-prebuilt boost-system-prebuilt boost-regex-prebuilt boost-python-prebuilt lapack blas f2c vhcl vhmsg bonebus iconv-prebuilt pprAI steerlib ode  openal alut tremolo sndfile proto-prebuilt $(CEREVOICE_LIBS)
LOCAL_SHARED_LIBRARIES := python-prebuilt-share 
include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)








