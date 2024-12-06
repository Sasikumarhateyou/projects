from django.db import models

# Create your models here.

class register(models.Model):
    reg_user=models.CharField(max_length=20)
    reg_pass=models.CharField(max_length=10)

    def __str__(self):
        return self.reg_user
