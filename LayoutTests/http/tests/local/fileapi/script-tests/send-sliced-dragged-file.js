description("Test for slicing a dragged file and sending it via XMLHttpRequest.");

var tempFileContent = "1234567890";
var tempFileName = "send-slice-dragged-file.tmp";
var subfile;

function uploadFile(file, filePath, start, length, expectedException)
{
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "http://127.0.0.1:8000/resources/post-and-verify.cgi?path=" + filePath + "&start=" + start + "&length=" + length, false);

    var passed;
    var message;
    try {
        xhr.send(file);
        if (expectedException) {
            passed = false;
            message = "Unexpected response data received: " + xhr.responseText + ". Expecting exception thrown";
        } else {
            if (xhr.responseText == "OK") {
                passed = true;
                message = "Expected response data received: " + xhr.responseText;
            } else {
                passed = false;
                message = "Unexpected response data received: " + xhr.responseText;
            }
        }
    } catch (ex) {
        if (expectedException) {
            passed = true;
            message = "Expected exception thrown: " + ex;
        } else {
            passed = false;
            message = "Unexpected exception thrown: " + ex;
        }
    }

    if (passed)
        testPassed(message);
    else
        testFailed(message);
}

function computeExpectedLength(fileLength, start, length)
{
    var expectedLength = length;
    if (start + length > fileLength) {
        if (start >= fileLength)
            expectedLength = 0;
        else
            expectedLength = fileLength - start;
    }
    return expectedLength;
}

function onStableFileDrop(file, filePath, fileLength, start, length, contentType)
{
    // Slice the file.
    subfile = file.webkitSlice(start, start + length, contentType);
    shouldEvaluateTo("subfile.size", computeExpectedLength(fileLength, start, length));
    shouldBe("subfile.type", (contentType != undefined && contentType != null) ? '"' + contentType + '"' : '""');

    // Upload the sliced file.
    uploadFile(subfile, filePath, start, length, false);
}

function dragAndSliceStableFile(filePath, fileLength, start, length, contentType)
{
    setFileInputDropCallback(function(file) { onStableFileDrop(file, "../local/fileapi/" + filePath, fileLength, start, length, contentType); });
    eventSender.beginDragWithFiles([filePath]);
    moveMouseToCenterOfElement(fileInput);
    eventSender.mouseUp();
}

function onUnstableFileDrop(file, filePath, fileLength, start, length, contentType)
{
    // Slice the file.
    subfile = file.webkitSlice(start, start + length, contentType);
    shouldEvaluateTo("subfile.size", computeExpectedLength(fileLength, start, length));
    shouldBe("subfile.type", (contentType != undefined && contentType != null) ? '"' + contentType + '"' : '""');
  
    // Upload the sliced file.
    uploadFile(subfile, filePath, start, length, false);

    // Touch the underlying temp file.
    touchTempFile(tempFileName);
    
    // Upload the sliced file. We should receive an exception since the file has been changed.
    uploadFile(subfile, filePath, start, length, true);

    // Remove the temp file.
    removeTempFile(tempFileName);
}

function dragAndSliceUnstableFile(start, length, contentType)
{
    var tempFilePath = createTempFile(tempFileName, tempFileContent);
    if (tempFilePath.length == 0)
        return;

    setFileInputDropCallback(function(file) { onUnstableFileDrop(file, tempFilePath, tempFileContent.length, start, length, contentType); });
    eventSender.beginDragWithFiles([tempFilePath]);
    moveMouseToCenterOfElement(fileInput);
    eventSender.mouseUp();
}

function runTest()
{
    debug("Test slicing and sending an empty file.");
    dragAndSliceStableFile("../resources/empty.txt", 0, 0, 10);

    debug("Test slicing and sending a small file.");
    dragAndSliceStableFile("../resources/file-for-drag-to-send.txt", 10, 2, 4, null);
    dragAndSliceStableFile("../resources/file-for-drag-to-send.txt", 10, 2, 20, "type/foo");
    dragAndSliceStableFile("../resources/file-for-drag-to-send.txt", 10, 15, 20, "type/bar");

    // This is to test a file that exceeds the read buffer limit (2K in Mac).
    debug("Test slicing and sending a big file.");
    dragAndSliceStableFile("../resources/abe.png", 12242, 10, 40);
    dragAndSliceStableFile("../resources/abe.png", 12242, 10, 3000);
    dragAndSliceStableFile("../resources/abe.png", 12242, 3000, 15000);

    debug("Test slicing and sending a file that has been changed right before sending.");
    dragAndSliceUnstableFile(3, 5);
}

if (window.eventSender) {
    runTest();
    // Clean up after ourselves
    removeFileInputElement();
} else {
    testFailed("This test is not interactive, please run using DumpRenderTree");
}

var successfullyParsed = true;
