$('#infotabs').tabs();
$('#record-button').button();

var benchmarkName = "Viewpoint Control";
view.ontouchmove = evaluateViewpoint;
view.onmousedrag = evaluateViewpoint;
view.onmousewheel = evaluateViewpoint;

function evaluateViewpoint(event) {
  var camera = view.x3dScene.viewpoint.camera;
  var currentOrientation = webots.quaternionToAxisAngle(camera.quaternion);
  currentOrientation.axis.negate();
  var targetPosition = new THREE.Vector3(0, 0.45, 0.333);
  var targetOrientation = { 'axis': new THREE.Vector3(0, 0.7071, 0.7071), 'angle': Math.PI };
  var maxIndex;
  if (Math.abs(currentOrientation.axis.x) > Math.abs(currentOrientation.axis.y)) {
    if (Math.abs(currentOrientation.axis.x) > Math.abs(currentOrientation.axis.z))
      maxIndex = 'x';
    else
      maxIndex = 'z';
  } else if (Math.abs(currentOrientation.axis.y) > Math.abs(currentOrientation.axis.z))
    maxIndex = 'y';
  else
    maxIndex = 'z';
  if (currentOrientation[maxIndex] < 0) {
    currentOrientation.axis.negate();
    currentOrientation.angle = -currentOrientation.angle;
  }
  var positionDifference = camera.position.distanceTo(targetPosition);
  var orientationDifference = currentOrientation.axis.distanceToSquared(targetOrientation.axis);
  if (currentOrientation.angle < 0)
    currentOrientation.angle += 2 * Math.PI;
  var angleDifference = Math.abs(currentOrientation.angle - targetOrientation.angle);
  performance = angleDifference + orientationDifference + positionDifference;
  performance = 100 - performance * 20
  if (performance < 0)
    performance = 0;
  performance -= 90;
  performance *= 10;
  var performanceString = "<font color='";
  if (performance > 0) {
    performanceString += 'green';
    if ($('#record-button').attr('disabled')) {
      $('#record-button').attr('disabled', false).removeClass('ui-state-disabled');
      $('#record-button').css('font-weight', 'bold');
    }
  } else {
    performanceString += 'red';
    if (!$('#record-button').attr('disabled')) {
      $('#record-button').attr('disabled', true).addClass('ui-state-disabled');
      $('#record-button').css('font-weight', 'normal');
    }
  }
  performanceString += "'>" + performance.toFixed(2) + "%</font>";
  $('#achievement').html(performanceString);
}

function recordPerformance() {
  var p = performance / 100;
  saveCookies(benchmarkName, parseFloat(p.toFixed(4)));
  var email = getCookie('email');
  var password = getCookie('password');
  if (email === undefined || password === undefined) {
    webots.alert(benchmarkName + " complete.",
                 performance.toFixed(2) + "% complete<br>" +
                 "<p>You should log in at <a href='https://robotbenchmark.net'>robotbenchmark.net</a> to record your performance.</p>");
    return false;
  }
  email = decodeURIComponent(email);
  $.post('/record.php', {email: email,
                         password: password,
                         benchmark: 'viewpoint_control',
                         record: performance / 100,
                         key: '1'}).done(function(data) {
                           if (data.startsWith('OK:'))
                             showBenchmarkRecord('record:' + data, benchmarkName, metricToString);
                           else
                             showBenchmarkError('record:' + data, benchmarkName);
                         });
  return true;
}

function metricToString(metric) {
  return (metric * 100).toFixed(2) + '%';
}
