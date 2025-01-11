
-- table for our functions
local InsightLFS = { }-

-- Check if a file or directory exists in this path
function InsightLFS.FileExists(file)
    local ok, err, code = os.rename(file, file)
    if not ok then
       if code == 13 then
          --print "Permission denied, but it exists"
          return true
       elseif code == 5 then
        --print "Permission denied, but it exists"
        return true
       end
    end
    return false
 end
 
 --- Check if a directory exists in this path
 function InsightLFS.Isdir(path)
    -- "/" works on both Unix and Windows
    local result = fileExists(path)
    return result
 end

 return InsightLFS