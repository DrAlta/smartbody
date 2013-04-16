import math
import random
print "|--------------------------------------------|"
print "|         Starting Ogre Demo           |"
print "|--------------------------------------------|"

scene.setScale(0.1)
# Add asset paths
scene.addAssetPath('script', 'sbm-common/scripts')
#scene.addAssetPath('script', 'sbm-common/scripts/behaviorsets')
scene.addAssetPath('script', 'behaviorsets')
scene.addAssetPath('mesh', 'mesh')
#scene.addAssetPath('mesh', 'retarget/mesh')
scene.addAssetPath('motion', 'Ogre')
scene.addAssetPath('motion', 'ChrBrad')
#scene.addAssetPath('motion', 'retarget/motion')
scene.addAssetPath('motion', 'sbm-common/common-sk')
scene.loadAssets()

# Set scene parameters and camera
#scene.getPawn('camera').setPosition(SrVec(0, -5, 0))

# Set joint map for Sinbad
print 'Setting up joint map for Brad'
scene.run('ogre-sinbad-map.py')
sinbadSkName = 'Sinbad.skeleton.xml'
jointMapManager = scene.getJointMapManager()
sinbadMap = jointMapManager.getJointMap('Sinbad.skeleton.xml')
ogreSk = scene.getSkeleton(sinbadSkName)
sinbadMap.applySkeleton(ogreSk)

# Behavior set setup
scene.run('behaviorsetup.py')

# Animation setup
#scene.run('init-param-animation.py')
steerManager = scene.getSteerManager()

# Setting up Sinbad
print 'Setting up Sinbad'
sinbadName = 'sinbad'
sinbad = scene.createCharacter(sinbadName,'')
sinbadSk = scene.createSkeleton(sinbadSkName)
sinbad.setSkeleton(sinbadSk)
sinbadPos = SrVec(0,5.16, 0)
sinbad.setPosition(sinbadPos)
sinbad.createStandardControllers()
sinbad.setStringAttribute('deformableMesh', 'Sinbad')
scene.run('BehaviorSetMaleLocomotion.py')
setupBehaviorSet()
retargetBehaviorSet(sinbadName,sinbadSkName)
scene.command('char sinbad viewer deformableGPU')

print 'Configuring scene parameters and camera'
scene.setBoolAttribute('internalAudio', True)
scene.run('default-viewer.py')
camera = getCamera()
camera.setEye(0, 5.98, 13.44)
camera.setCenter(1.0, 1.7, -39.5)
camera.setUpVector(SrVec(0, 1, 0))
camera.setScale(1)
camera.setFov(1.0472)
camera.setFarPlane(100)
camera.setNearPlane(0.1)
camera.setAspectRatio(1.02)

sim.start()
bml.execBML(sinbadName, '<body posture="ChrUtah_Idle001"/>')
sim.resume()
