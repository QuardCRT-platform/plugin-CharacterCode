param (
    [string]$cc_info = "mingw",
    [string]$tpath = "./plugininterface/plugininterface.json.temp",
    [string]$opath = "./plugininterface/plugininterface.json"
)

# copy file $tpath to $opath
Copy-Item $tpath $opath
# replace file #COMPLILER_INFO# to $cc_info
(Get-Content $opath) | Foreach-Object {$_ -replace "#COMPLILER_INFO#", $cc_info} | Set-Content $opath
