print "== Beginning: data/sbm-common/scripts/init-common-assets.py"


###  Assumes current directory is: core/smartbody/sbm/bin
scene.addAssetPath('ME', '../../../../data/sbm-common/common-sk')

print ">>> Loading common motions and poses..."
scene.loadAssets()
print "Completed: data/sbm-common/scripts/init-common-assets.py"
